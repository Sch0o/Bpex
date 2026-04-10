// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletManagerComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Players/ShooterPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Weapon/LagCompensation/LagCompensationSubsystem.h"


UBulletManagerComponent::UBulletManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	//子弹模拟应在物理之后
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetIsReplicatedByDefault(true);
}

float UBulletManagerComponent::GetWorldGravity() const
{
	return GetWorld() ? FMath::Abs(GetWorld()->GetGravityZ()) : 980.f;
}

void UBulletManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		LagCompSubsystem = World->GetSubsystem<ULagCompensationSubsystem>();
	}
}

void UBulletManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (FActiveBullet& Bullet : ActiveBullets)
	{
		if (!Bullet.bPendingKill)
		{
			SimulateBullet(Bullet, DeltaTime);
		}
	}
	CleanupDeadBullets();
}

//发射子弹，对外接口
int32 UBulletManagerComponent::FireBullet(const UBulletDataAsset* Config, const FVector& Origin,
                                          const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpec)
{
	
	if (!IsValid(Config) || ActiveBullets.Num() >= MaxActiveBullets)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBulletManagerComponent::FireBullet:: config is null or bullets num over max"));
		return 0;
	}
	const uint32 BulletID = ++NextBulletID;
	const FVector NormalDir = Direction.GetSafeNormal();
	const bool bIsServer = GetOwnerRole() == ROLE_Authority;
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const bool bIsLocallyControlled =
		OwnerPawn ? Cast<APawn>(GetOwner())->IsLocallyControlled() : false;

	//创建子弹实例
	FActiveBullet NewBullet;
	NewBullet.StartPosition = Origin;
	NewBullet.Position = Origin;
	NewBullet.Velocity = NormalDir * Config->Speed;
	NewBullet.Config = Config;
	NewBullet.BulletID = BulletID;
	NewBullet.bIsAuthority = bIsServer;
	NewBullet.DamageSpecHandle = DamageSpec;
	NewBullet.LifeRemaining = Config->MaxLifetime;
	NewBullet.bIsLocalPrediction = !bIsServer && bIsLocallyControlled;
	NewBullet.Instigator = GetOwner();
	NewBullet.InstigatorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	ActiveBullets.Add(NewBullet);

	if (bIsLocallyControlled)
	{
		SpawnTracer(ActiveBullets.Last());
	}

	if (!bIsServer && bIsLocallyControlled)
	{
		FFireParams Params;
		Params.Origin = Origin;
		Params.Direction = NormalDir;
		Params.BulletID = BulletID;

		if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(OwnerPawn->GetController()))
		{
			Params.ServerTime = PC->GetServerTime();
		}

		Server_FireBullet(Params, Config);
	}

	//通知其它客户端显示弹道特效
	if (bIsServer)
	{
		Multicast_SpawnTracerVFX(Origin, NormalDir, Config);
	}
	return BulletID;
}

void UBulletManagerComponent::Client_DrawServerTrajectory_Implementation(FVector ServerStart, FVector ServerEnd)
{
	DrawDebugLine(GetWorld(), ServerStart, ServerEnd, FColor::Red, false, 2.0f, 0, 1.0f);
}

bool UBulletManagerComponent::PerformBulletTrace(FActiveBullet& Bullet, const FVector& Start, const FVector& End,
                                                 FHitResult& OutHit, float RewindToTime)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	//射出子弹后角色可能销毁
	if (Bullet.Instigator.IsValid())
	{
		QueryParams.AddIgnoredActor(Bullet.Instigator.Get());
	}
	const bool bShouldRewind = Bullet.bIsAuthority && bEnableTargetRewind &&
		LagCompSubsystem && RewindToTime > 0.f;

	if (bShouldRewind)
	{
		if (Bullet.Config->CollisionRadius > 0.f)
		{
			return LagCompSubsystem->RewindSweep(OutHit, Start, End,
			                                     FCollisionShape::MakeSphere(Bullet.Config->CollisionRadius),
			                                     BulletTraceChannel, QueryParams,
			                                     RewindToTime, Bullet.Instigator.Get());
		}
		else
		{
			return LagCompSubsystem->RewindLineTrace(
				OutHit, Start, End,
				BulletTraceChannel, QueryParams,
				RewindToTime, Bullet.Instigator.Get());
		}
	}

	if (!Bullet.bIsLocalPrediction && !Bullet.bIsAuthority)
	{
		//纯视觉子弹，廉价检测
		QueryParams.bTraceComplex = false;
	}
	if (Bullet.Config && Bullet.Config->CollisionRadius > 0.f)
	{
		return GetWorld()->SweepSingleByChannel(
			OutHit, Start, End, FQuat::Identity, BulletTraceChannel,
			FCollisionShape::MakeSphere(Bullet.Config->CollisionRadius), QueryParams);
	}
	else
	{
		return GetWorld()->LineTraceSingleByChannel(
			OutHit, Start, End, BulletTraceChannel, QueryParams);
	}
}

void UBulletManagerComponent::FastForwardBullet(FActiveBullet& Bullet, float CatchUpTime)
{
	if (!IsValid(Bullet.Config) || CatchUpTime <= 0.f)return;
	if (!LagCompSubsystem)return;
	const float Gravity = GetWorldGravity() * Bullet.Config->GravityScale;
	const float SubStepSize = 1.f / 60.f;
	float RemainingTime = CatchUpTime;
	float SimTime = Bullet.ServerTime;
	while (RemainingTime > 0.f && !Bullet.bPendingKill)
	{
		const float StepDelta = FMath::Min(RemainingTime, SubStepSize);
		const FVector OldPosition = Bullet.Position;

		// 推进位置
		Bullet.Velocity.Z -= Gravity * StepDelta;
		Bullet.Position += Bullet.Velocity * StepDelta;

		// 更新飞行状态
		const float StepDist = FVector::Dist(OldPosition, Bullet.Position);
		Bullet.DistanceTraveled += StepDist;
		Bullet.LifeRemaining -= StepDelta;

		if (Bullet.LifeRemaining <= 0.f || Bullet.DistanceTraveled >= Bullet.Config->MaxDistance)
		{
			Bullet.bPendingKill = true;
			return;
		}

		FHitResult HitResult;

		bool bHit = PerformBulletTrace(Bullet, OldPosition, Bullet.Position, HitResult, SimTime);

		if (bHit)
		{
			Bullet.Position = HitResult.ImpactPoint;
			ProcessHit(Bullet, HitResult);
			Bullet.bPendingKill = true;
			break;
		}
		//推进模拟时间
		SimTime += StepDelta;
		RemainingTime -= StepDelta;
	}
}


void UBulletManagerComponent::SimulateBullet(FActiveBullet& Bullet, float DeltaTime)
{
	if (!IsValid(Bullet.Config))
	{
		return;
	}
	const FVector OldPosition = Bullet.Position;

	//重力影响
	const float Gravity = GetWorldGravity() * Bullet.Config->GravityScale;
	Bullet.Position.Z -= Gravity * DeltaTime;
	Bullet.Position += Bullet.Velocity * DeltaTime;

	//更新飞行距离
	const float StepDistance = FVector::Dist(OldPosition, Bullet.Position);
	Bullet.DistanceTraveled += StepDistance;
	//更新生命周期
	Bullet.LifeRemaining -= DeltaTime;
	if (Bullet.LifeRemaining <= 0.0f || Bullet.DistanceTraveled >= Bullet.Config->MaxDistance)
	{
		Bullet.bPendingKill = true;
		return;
	}

	//碰撞检测
	FHitResult HitResult;
	float RewindToTime = GetWorld()->GetTimeSeconds() - Bullet.OwnerHalfRTT;
	bool bHit = PerformBulletTrace(Bullet, OldPosition, Bullet.Position, HitResult, RewindToTime);

	if (bHit)
	{
		ProcessHit(Bullet, HitResult);
		Bullet.bPendingKill = true;
		return;
	}
	UpdateTracer(Bullet);

	// if (Bullet.bIsAuthority)
	// {
	// 	// 2. 调用 RPC，把坐标通过网络塞给客户端
	// 	Client_DrawServerTrajectory(OldPosition, Bullet.Position);
	// }
	//
	// if (Bullet.bIsLocalPrediction)
	// {
	// 	// 专服不画（没视口也没意义）
	// 	// PIE 中的伪专服也不画（避免混淆）
	// 	if (GetNetMode() != NM_DedicatedServer)
	// 	{
	// 		APawn* OwnerPawn = Cast<APawn>(GetOwner());
	// 		if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	// 		{
	// 			FColor LineColor = Bullet.bIsAuthority ? FColor::Red : FColor::Yellow;
	// 			DrawDebugLine(GetWorld(), OldPosition, Bullet.Position,
	// 			              LineColor, false, 2.0f, 0, 0.5f);
	// 		}
	// 	}
	// }
}

void UBulletManagerComponent::ProcessHit(FActiveBullet& Bullet, const FHitResult& HitResult)
{
	//施加命中特效
	if (Bullet.Config && Bullet.Config->ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), Bullet.Config->ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	//广播命中
	OnBulletHit.Broadcast(HitResult, Bullet);

	//伤害应用，只在服务器端
	if (!Bullet.bIsAuthority)return;
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!TargetASC) return;
	if (Bullet.DamageSpecHandle.IsValid())
	{
		// 将碰撞信息注入 EffectContext（用于后续判断爆头等）
		FGameplayEffectContextHandle Context = Bullet.DamageSpecHandle.Data->GetEffectContext();
		Context.AddHitResult(HitResult);
		TargetASC->ApplyGameplayEffectSpecToSelf(*Bullet.DamageSpecHandle.Data.Get());
	}
	else if (Bullet.InstigatorASC.IsValid() && Bullet.Config->DamageEffectClass)
	{
		// 备用路径：动态构建 Spec
		FGameplayEffectContextHandle Context =
			Bullet.InstigatorASC->MakeEffectContext();
		Context.AddInstigator(Bullet.Instigator.Get(), Bullet.Instigator.Get());
		Context.AddHitResult(HitResult);
		FGameplayEffectSpecHandle Spec = Bullet.InstigatorASC->MakeOutgoingSpec(
			Bullet.Config->DamageEffectClass, 1, Context);
		if (Spec.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void UBulletManagerComponent::CleanupDeadBullets()
{
	for (int32 i = ActiveBullets.Num() - 1; i >= 0; --i)
	{
		if (ActiveBullets[i].bPendingKill)
		{
			if (ActiveBullets[i].TracerComponent.IsValid())
			{
				ActiveBullets[i].TracerComponent->Deactivate();
			}
			ActiveBullets.RemoveAtSwap(i);
		}
	}
}

void UBulletManagerComponent::SpawnTracer(FActiveBullet& Bullet)
{
	if (!Bullet.Config || !Bullet.Config->TracerSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBulletManagerComponent::SpawnTracer:: config or tracerSystem is null"));
		return;
	}
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), Bullet.Config->TracerSystem, Bullet.Position, Bullet.Velocity.Rotation(), FVector(1.f), true, true,
		ENCPoolMethod::AutoRelease);
	if (NiagaraComponent)
	{
		Bullet.TracerComponent = NiagaraComponent;
	}
}

void UBulletManagerComponent::UpdateTracer(FActiveBullet& Bullet)
{
	if (Bullet.TracerComponent.IsValid())
	{
		Bullet.TracerComponent->SetWorldLocation(Bullet.Position);
		Bullet.TracerComponent->SetWorldRotation(Bullet.Velocity.Rotation());
	}
}

void UBulletManagerComponent::Multicast_SpawnTracerVFX_Implementation(FVector Origin, FVector Direction,
                                                                      const UBulletDataAsset* Config)
{
	if (GetNetMode() == NM_DedicatedServer) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		return;
	}
	if (!Config || !Config->TracerSystem) return;

	FActiveBullet VisualBullet;
	VisualBullet.Position = Origin;
	VisualBullet.StartPosition = Origin;
	VisualBullet.Velocity = Direction * Config->TracerSpeed;
	VisualBullet.Config = Config;
	VisualBullet.LifeRemaining = Config->MaxLifetime;
	VisualBullet.bIsAuthority = false;
	VisualBullet.bIsLocalPrediction = false;
	VisualBullet.BulletID = 0; // 纯视觉，不需要ID
	ActiveBullets.Add(VisualBullet);
	SpawnTracer(ActiveBullets.Last());
}

void UBulletManagerComponent::Server_FireBullet_Implementation(FFireParams Params, const UBulletDataAsset* Config)
{
	//服务器创建权威子弹
	FActiveBullet AuthoritativeBullet;
	AuthoritativeBullet.Position = Params.Origin;
	AuthoritativeBullet.StartPosition = Params.Origin;
	AuthoritativeBullet.Velocity = Params.Direction * Config->Speed;
	AuthoritativeBullet.Config = Config;
	AuthoritativeBullet.LifeRemaining = Config->MaxLifetime;
	AuthoritativeBullet.Instigator = GetOwner();
	AuthoritativeBullet.BulletID = Params.BulletID;
	AuthoritativeBullet.bIsAuthority = true;
	AuthoritativeBullet.bIsLocalPrediction = false;
	AuthoritativeBullet.OwnerHalfRTT = GetOwnerHalfRTT();
	AuthoritativeBullet.ServerTime = Params.ServerTime;
	

	UAbilitySystemComponent* ASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	if (ASC && Config->AmmoCostEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle CostSpec = ASC->MakeOutgoingSpec(
			Config->AmmoCostEffectClass, 1, Context);
		ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get());
	}
	if (ASC && Config->DamageEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddInstigator(GetOwner(), GetOwner());
		AuthoritativeBullet.DamageSpecHandle =
			ASC->MakeOutgoingSpec(Config->DamageEffectClass, 1, Context);
		AuthoritativeBullet.InstigatorASC = ASC;
	}

	const float ServerNow = GetWorld()->GetTimeSeconds();
	float CatchUpTime = ServerNow - Params.ServerTime;
	UE_LOG(LogTemp, Warning,TEXT("ServeNow %.4fs, Params.ServerTime %.4fs"),ServerNow,Params.ServerTime);
	CatchUpTime = FMath::Clamp(CatchUpTime, 0.f, MaxCatchUpTime);
	if (CatchUpTime > KINDA_SMALL_NUMBER)
	{
		FastForwardBullet(AuthoritativeBullet, CatchUpTime);
		UE_LOG(LogTemp, Warning,
		       TEXT("Bullet %d: FastForward %.1fms, HalfRTT %.1fms"),
		       Params.BulletID,
		       CatchUpTime * 1000.f,
		       AuthoritativeBullet.OwnerHalfRTT * 1000.f);
	}
	if (!AuthoritativeBullet.bPendingKill)
	{
		ActiveBullets.Add(AuthoritativeBullet);
	}

	Multicast_SpawnTracerVFX(Params.Origin, Params.Direction, Config);
}

bool UBulletManagerComponent::Server_FireBullet_Validate(FFireParams Params, const UBulletDataAsset* Config)
{
	if (!Config) return false;
	//── 防作弊验证 ──
	AActor* Owner = GetOwner();
	if (!Owner) return false;
	// 发射点不能离玩家太远（枪口距身体的合理范围）
	float DistFromOwner = FVector::Dist(Params.Origin, Owner->GetActorLocation());
	if (DistFromOwner > 300.f)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("BulletManager: Fire origin too far from player (%.1f)"),
		       DistFromOwner);
		return false;
	}
	// 方向必须是合理的单位向量
	if (!Params.Direction.IsNormalized())
	{
		return false;
	}
	return true;
}

float UBulletManagerComponent::GetOwnerHalfRTT() const
{
	constexpr float MaxPredictionPing = 500;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return 0.f;
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC || !PC->PlayerState) return 0.f;
	const float PingMs = FMath::Clamp(PC->PlayerState->ExactPing, 0, MaxPredictionPing);
	return PingMs * 0.001f * 0.5f;
}
