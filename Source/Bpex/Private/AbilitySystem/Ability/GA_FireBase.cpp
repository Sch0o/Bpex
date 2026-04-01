// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_FireBase.h"

#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterProjectile.h"
#include "Weapon/ShooterWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UGA_FireBase::UGA_FireBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 网络预测
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_FireBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, false, nullptr))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//仅仅查看，但不施加效果
	if (!CheckCost(Handle, ActorInfo, nullptr))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	AutoFireTick();
	
	GetWorld()->GetTimerManager().SetTimer(AutoFireTimerHandle,this,&UGA_FireBase::AutoFireTick,TimeBetweenShots,true);
}

void UGA_FireBase::AutoFireTick()
{
	if (!CheckCost(CurrentSpecHandle, CurrentActorInfo, nullptr))
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
		K2_OnInputReleased();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	CommitAbilityCost(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,nullptr);
	
	FGameplayCueParameters Params;
	Params.Instigator = GetAvatarActorFromActorInfo();
	if (FireCueTag.IsValid())
	{
		GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(FireCueTag, Params);
	}
	
	PerformHitscan();
}

void UGA_FireBase::PerformHitscan()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		return;
	}
	UCombatComponent* CombatComponent = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!CombatComponent)
	{
		return;
	}
	AShooterWeapon* Weapon = CombatComponent->GetCurrentWeapon();
	if (!Weapon)return;

	FVector TargetLocation = CombatComponent->GetWeaponTargetLocation();
	FTransform SpawnTransform = Weapon->CalculateProjectileSpawnTransform(TargetLocation);

	FVector TraceStart = SpawnTransform.GetLocation(); // 枪口位置
	FVector Direction = (TargetLocation - TraceStart).GetSafeNormal();
	FVector TraceEnd = TraceStart + (Direction * HitscanRange);

	// 2. 设置射线检测参数
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor); // 忽略玩家自己
	QueryParams.AddIgnoredActor(Weapon); // 忽略武器本身
	QueryParams.bTraceComplex = true; // 使用复杂碰撞以获得更精确的命中（比如精准爆头）

	// 3. 执行射线检测
	FHitResult HitResult;
	// 注意：ECC_Visibility 是默认通道，你可以根据项目设置换成专用的射击通道 (比如 ECC_GameTraceChannel1)
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// 【可选】Debug 射线：帮你直观看到子弹弹道
	DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, FColor::Red, false, 2.0f);

	// 4. 如果命中了，并且在服务器端（Server），则应用伤害
	if (bHit && HasAuthority(&CurrentActivationInfo))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 尝试获取受击者的 Ability System Component
			UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

			if (TargetASC && DamageEffectClass)
			{
				// 创建 Effect Context，并将 HitResult 放进去（这对后续读取受击部位、播放特效很有用）
				FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->
					MakeEffectContext();
				EffectContext.AddInstigator(AvatarActor, AvatarActor);
				EffectContext.AddHitResult(HitResult);

				// 生成并应用伤害 Spec
				FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
					DamageEffectClass, GetAbilityLevel(), EffectContext);
				TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			}
		}
	}
}

void UGA_FireBase::SpawnProjectile()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		return;
	}
	UCombatComponent* CombatComponent = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!CombatComponent)
	{
		return;
	}

	FVector TargetLocation = CombatComponent->GetWeaponTargetLocation();
	AShooterWeapon* Weapon = CombatComponent->GetCurrentWeapon();
	if (!Weapon) return;
	FTransform ProjectileTransform = Weapon->CalculateProjectileSpawnTransform(TargetLocation);

	if (ProjectileTransform.IsValid() && ProjectileClass && HasAuthority(&CurrentActivationInfo))
	{
		FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		EffectContext.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
			DamageEffectClass, GetAbilityLevel(), EffectContext);

		AShooterProjectile* Projectile = GetWorld()->SpawnActorDeferred<AShooterProjectile>(
			ProjectileClass,
			ProjectileTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetAvatarActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);
		if (Projectile)
		{
			Projectile->DamageEffectSpecHandle = DamageSpecHandle;
			Projectile->FinishSpawning(ProjectileTransform);
		}
	}
}


void UGA_FireBase::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (IsActive())
	{
		K2_OnInputReleased();
	}
}
