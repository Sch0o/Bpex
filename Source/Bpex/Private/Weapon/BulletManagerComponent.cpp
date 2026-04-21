// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletManagerComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
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

	//模拟本地子弹
	for (FActiveBullet& Bullet : ActiveBullets)
	{
		if (!Bullet.bPendingKill)
		{
			SimulateBullet(Bullet, DeltaTime);
		}
	}
	//清除废弃子弹
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

	if (bIsServer && !bIsLocallyControlled) return 0;

	//创建子弹实例
	FActiveBullet NewBullet;
	NewBullet.StartPosition = Origin;
	NewBullet.Position = Origin;
	NewBullet.Velocity = NormalDir * Config->Speed;
	NewBullet.StartVelocity= NormalDir*Config->Speed;
	NewBullet.Config = Config;
	NewBullet.BulletID = BulletID;
	NewBullet.DamageSpecHandle = DamageSpec;
	NewBullet.LifeRemaining = Config->MaxLifetime;
	NewBullet.Instigator = GetOwner();
	NewBullet.InstigatorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	NewBullet.bIsAuthority = bIsServer;
	NewBullet.bIsLocalPrediction = (!bIsServer && bIsLocallyControlled);
	// 记录开火时间
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(OwnerPawn->GetController()))
	{
		NewBullet.FireServerTime = PC->GetServerTime();
	}
	
	ActiveBullets.Add(NewBullet);

	//如果本地，播放子弹特效
	if (bIsLocallyControlled)
	{
		SpawnTracer(ActiveBullets.Last());
	}

	if (!bIsServer)
	{
		FFireParams Params;
		Params.Origin = Origin;
		Params.Direction = NormalDir;
		Params.BulletID = BulletID;
		Params.ServerTime = NewBullet.FireServerTime;

		Server_NotifyFire(Params, Config);
	}
	else
	{
		//通知其它客户端显示弹道特效
		Multicast_SpawnTracerVFX(Origin, NormalDir, Config);
	}

	return BulletID;
}

void UBulletManagerComponent::Server_ReportHit_Implementation(
	FBulletHitReport Report, const UBulletDataAsset* Config)
{
	if (!IsValid(Config) || !Report.HitActor.IsValid()) return;
	const float HalfRTT = GetOwnerHalfRTT() * 2;
	const float ClampedRewind = FMath::Min(HalfRTT, MaxCatchUpTime);
	// ① 回溯目标HitBox
	bool bRewound = false;
	if (bEnableTargetRewind && LagCompSubsystem && ClampedRewind > KINDA_SMALL_NUMBER)
	{
		bRewound = LagCompSubsystem->RewindActor(
			Report.HitActor.Get(), ClampedRewind);
	}
	// ② 在回溯状态下模拟弹道验证
	FHitResult ValidatedHit;
	const bool bValid = ServerValidateHit(Report, Config, ValidatedHit);
	// ③ 立即恢复
	if (bRewound && LagCompSubsystem)
	{
		LagCompSubsystem->RestoreActor(Report.HitActor.Get());
	}
	// ④ 处理结果
	if (bValid)
	{
		ServerApplyDamage(Report, Config, ValidatedHit);
	}
}

bool UBulletManagerComponent::ServerValidateHit(
	const FBulletHitReport& Report,
	const UBulletDataAsset* Config,
	FHitResult& OutValidatedHit)
{
	if (!IsValid(Config))
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerValidateHit: Invalid Config, BulletID=%u"),
		       Report.BulletID);
		return false;
	}
	// ── 0. 基础合法性检查 ──
	// 起始速度方向必须合理
	const FVector StartDir = Report.StartVelocity.GetSafeNormal();
	if (StartDir.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: Zero velocity, BulletID=%u"),
		       Report.BulletID);
		return false;
	}
	// 起始速度大小不能偏离配置太多（允许5%误差，防浮点精度问题）
	const float ReportedSpeed = Report.StartVelocity.Size();
	const float ExpectedSpeed = Config->Speed;
	if (FMath::Abs(ReportedSpeed - ExpectedSpeed) / ExpectedSpeed > 0.05f)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: Speed mismatch (%.1f vs %.1f), BulletID=%u"),
		       ReportedSpeed, ExpectedSpeed, Report.BulletID);
		return false;
	}
	// 起始位置不能离拥有者太远
	AActor* Owner = GetOwner();
	if (Owner)
	{
		const float OriginDist =
			FVector::Dist(Report.StartLocation, Owner->GetActorLocation());
		if (OriginDist > MaxOriginDeviation)
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("ServerValidateHit: Origin too far (%.1f), BulletID=%u"),
			       OriginDist, Report.BulletID);
			return false;
		}
	}
	// ── 1. 构建服务器模拟子弹 ──
	FActiveBullet SimBullet;
	SimBullet.StartPosition = Report.StartLocation;
	SimBullet.Position = Report.StartLocation;
	SimBullet.Velocity = Report.StartVelocity;
	SimBullet.Config = Config;
	SimBullet.LifeRemaining = Config->MaxLifetime;
	SimBullet.DistanceTraveled = 0.f;
	SimBullet.bPendingKill = false;
	SimBullet.bIsAuthority = true; // 服务器权威模拟
	SimBullet.bIsLocalPrediction = false;
	SimBullet.Instigator = GetOwner(); // 碰撞忽略自身
	// ── 2. 模拟弹道 ──
	constexpr float SimFrequency = 60.f;
	const float MaxSimTime = Config->MaxLifetime;
	TArray<FVector> PathPositions;
	FHitResult SimHit;
	const bool bSimHit = PredictCustomProjectilePath(
		SimBullet, MaxSimTime, SimFrequency, PathPositions, SimHit);
	// ── 3. 没命中任何东西 ──
	if (!bSimHit)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: No hit found in simulation, BulletID=%u"),
		       Report.BulletID);
		return false;
	}
	// ── 4. 验证命中的是同一个Actor ──
	AActor* SimHitActor = SimHit.GetActor();
	AActor* ReportedHitActor = Report.HitActor.Get();
	if (!SimHitActor || !ReportedHitActor)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: Null actor, BulletID=%u"),
		       Report.BulletID);
		return false;
	}
	if (SimHitActor != ReportedHitActor)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: Actor mismatch [Client=%s, Server=%s], BulletID=%u"),
		       *ReportedHitActor->GetName(),
		       *SimHitActor->GetName(),
		       Report.BulletID);
		return false;
	}
	// ── 5. 验证命中点容差 ──
	const float HitDeviation =
		FVector::Dist(SimHit.ImpactPoint, Report.ClientHitLocation);
	if (HitDeviation > HitValidationTolerance)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ServerValidateHit: Hit point deviation too large "
			       "(%.1f > %.1f), BulletID=%u"),
		       HitDeviation, HitValidationTolerance, Report.BulletID);
		return false;
	}
	// ── 6. 验证通过 ──
	OutValidatedHit = SimHit;
	UE_LOG(LogTemp, Log,
	       TEXT("ServerValidateHit: PASSED [Actor=%s, Deviation=%.1f, "
		       "SimSteps=%d], BulletID=%u"),
	       *SimHitActor->GetName(),
	       HitDeviation,
	       PathPositions.Num(),
	       Report.BulletID);
	return true;
}

void UBulletManagerComponent::ServerApplyDamage(const FBulletHitReport& Report, const UBulletDataAsset* Config,
                                                FHitResult ValidatedHit)
{
	AActor* HitActor = Report.HitActor.Get();
	//基础性检测
	if (!HitActor || !Config || !Config->DamageEffectClass)
	{
		return;
	}
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC) return;
	AActor* SourceActor = GetOwner();
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	if (!SourceASC) return;

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(SourceActor, SourceActor);
	EffectContext.AddHitResult(ValidatedHit);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Config->DamageEffectClass, 1.0f, EffectContext);

	if (SpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

bool UBulletManagerComponent::Server_ReportHit_Validate(FBulletHitReport Report, const UBulletDataAsset* Config)
{
	return true;
}

void UBulletManagerComponent::Server_NotifyFire_Implementation(FFireParams Params, const UBulletDataAsset* Config)
{
	UAbilitySystemComponent* ASC =
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	//扣除子弹
	if (ASC && Config->AmmoCostEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle CostSpec = ASC->MakeOutgoingSpec(
			Config->AmmoCostEffectClass, 1, Context);
		ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get());
	}
	//广播弹道特效给其他客户端
	Multicast_SpawnTracerVFX(Params.Origin, Params.Direction, Config);
}

void UBulletManagerComponent::Client_DrawServerTrajectory_Implementation(FVector ServerStart, FVector ServerEnd)
{
	DrawDebugLine(GetWorld(), ServerStart, ServerEnd, FColor::Red, false, 2.0f, 0, 1.0f);
}

bool UBulletManagerComponent::PerformBulletTrace(FActiveBullet& Bullet, const FVector& Start, const FVector& End,
                                                 FHitResult& OutHit, TEnumAsByte<ECollisionChannel> CollisionChannel)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;
	//射出子弹后角色可能销毁
	if (Bullet.Instigator.IsValid())
	{
		QueryParams.AddIgnoredActor(Bullet.Instigator.Get());
	}

	if (!Bullet.bIsLocalPrediction && !Bullet.bIsAuthority)
	{
		//纯视觉子弹，廉价检测
		QueryParams.bTraceComplex = false;
	}
	if (Bullet.Config && Bullet.Config->CollisionRadius > 0.f)
	{
		return GetWorld()->SweepSingleByChannel(
			OutHit, Start, End, FQuat::Identity, CollisionChannel,
			FCollisionShape::MakeSphere(Bullet.Config->CollisionRadius), QueryParams);
	}
	else
	{
		return GetWorld()->LineTraceSingleByChannel(
			OutHit, Start, End, CollisionChannel, QueryParams);
	}
}

bool UBulletManagerComponent::UpdateBulletParams(FActiveBullet& Bullet, float DeltaTime)
{
	if (!IsValid(Bullet.Config))
	{
		return false;
	}
	const FVector OldPosition = Bullet.Position;

	//重力影响
	const float Gravity = GetWorldGravity() * Bullet.Config->GravityScale;
	Bullet.Velocity.Z -= Gravity * DeltaTime;
	Bullet.Position += Bullet.Velocity * DeltaTime;

	//更新飞行距离和生命周期
	const float StepDistance = FVector::Dist(OldPosition, Bullet.Position);
	Bullet.DistanceTraveled += StepDistance;
	Bullet.LifeRemaining -= DeltaTime;
	if (Bullet.LifeRemaining <= 0.0f || Bullet.DistanceTraveled >= Bullet.Config->MaxDistance)
	{
		Bullet.bPendingKill = true;
		return false;
	}
	return true;
}

void UBulletManagerComponent::SimulateBullet(FActiveBullet& Bullet, float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("SimulateBullet"));

	FVector OldPosition = Bullet.Position;

	if (!UpdateBulletParams(Bullet, DeltaTime)) return;

	FHitResult HitResult;
	bool bHit = PerformBulletTrace(Bullet, OldPosition, Bullet.Position, HitResult, BulletTraceChannel);

	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimulateBullet: bHit = true"));
		ProcessLocalHit(Bullet, HitResult);
		Bullet.bPendingKill = true;
		return;
	}
	UpdateTracer(Bullet);
}

void UBulletManagerComponent::ProcessLocalHit(FActiveBullet& Bullet, const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ProcessLocalHit"));
	//施加命中特效
	if (Bullet.Config && Bullet.Config->ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), Bullet.Config->ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(),
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	//广播命中
	OnBulletHit.Broadcast(HitResult, Bullet);

	const bool bIsLocallyOwned = Bullet.bIsLocalPrediction || Bullet.bIsAuthority;
	if (!bIsLocallyOwned) return;

	//只有命中了有ASC的Actor才需要报告伤害
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!TargetASC) return;

	//构建命中报告
	FBulletHitReport Report;
	Report.BulletID = Bullet.BulletID;
	Report.HitActor = HitActor;
	Report.StartLocation = Bullet.StartPosition;
	Report.StartVelocity     = Bullet.StartVelocity;
	Report.ClientHitLocation = HitResult.ImpactPoint;

	if (Bullet.bIsAuthority)
	{
		UAbilitySystemComponent* SourceASC =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
		if (!SourceASC || !Bullet.DamageSpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("ProcessLocalHit: Invalid SourceASC or DamageSpec"));
			return;
		}
		// 把命中信息写入 Context
		FGameplayEffectContextHandle Context =
			Bullet.DamageSpecHandle.Data->GetEffectContext();
		Context.AddHitResult(HitResult);
		// ✅ 正确：SourceASC 对 TargetASC 施加效果
		SourceASC->ApplyGameplayEffectSpecToTarget(*Bullet.DamageSpecHandle.Data.Get(), TargetASC);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessLocalHit: call Server_ReportHit"));
		// 远程客户端：发RPC给服务端
		Server_ReportHit(Report, Bullet.Config);
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

bool UBulletManagerComponent::Server_NotifyFire_Validate(FFireParams Params, const UBulletDataAsset* Config)
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

bool UBulletManagerComponent::PredictCustomProjectilePath(
	const FActiveBullet& InitialBulletState,
	float MaxSimulateTime,
	float SimFrequency,
	TArray<FVector>& OutPathPositions,
	FHitResult& OutHit)
{
	OutPathPositions.Empty();

	if (!IsValid(InitialBulletState.Config))
	{
		return false;
	}

	FActiveBullet SimBullet = InitialBulletState;

	// 限制最小频率为1，防止除以0
	float StepDeltaTime = 1.0f / FMath::Max(SimFrequency, 1.0f);
	float AccumulatedTime = 0.0f;

	// 记录起点
	OutPathPositions.Add(SimBullet.Position);

	bool bHit = false;

	while (AccumulatedTime < MaxSimulateTime &&
	   SimBullet.LifeRemaining > 0.0f &&
	   SimBullet.DistanceTraveled < SimBullet.Config->MaxDistance)
	{
		const FVector OldPosition = SimBullet.Position;
		if (!UpdateBulletParams(SimBullet, StepDeltaTime))
		{
			break;  // 子弹已超时/超距
		}
		bHit = PerformBulletTrace(SimBullet, OldPosition, SimBullet.Position, OutHit, BulletTraceChannel);
		if (bHit)
		{
			OutPathPositions.Add(OutHit.Location);
			break;
		}
		else
		{
			OutPathPositions.Add(SimBullet.Position);
		}
		AccumulatedTime += StepDeltaTime;  // ✅ 累加时间
	}

	return bHit;
}
