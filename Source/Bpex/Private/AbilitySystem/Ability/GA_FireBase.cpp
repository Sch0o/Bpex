// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_FireBase.h"

#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterProjectile.h"
#include "Weapon/ShooterWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BpexGameplayTags.h"
#include "GameplayCueManager.h"
#include "Weapon/BulletManagerComponent.h"
#include "Weapon/BulletTypes.h"

UGA_FireBase::UGA_FireBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 网络预测
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	ActivationOwnedTags.AddTag(FBpexGameplayTags::Get().State_Action_Firing);
	
	SourceBlockedTags.AddTag(FBpexGameplayTags::Get().Ability_Weapon_NoFiring);
}

bool UGA_FireBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	const AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!AvatarActor)
	{
		return false;
	}
	const UCombatComponent* CombatComp = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!CombatComp || !CombatComp->HasWeaponEquipped())
	{
		return false;
	}
	return true;
}

void UGA_FireBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	if (!BulletConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("BulletConfig is NULL"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	CurrentSpreadAngle = 0.f;

	if (IsLocallyControlled())
	{
		InitLocalAmmoCount();
	}
	
	//调Super让蓝图接管
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_FireBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                              bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_FireBase::PerformFire()
{
	
	if (!IsLocallyControlled()) return false;
	
	if (!TryConsumeLocalAmmo())
	{
		UE_LOG(LogTemp, Warning, TEXT("NO Ammo"));
		return false;
	}
	
	FireSingleBullet();
	
	if (FireCueTag.IsValid())
	{
		FGameplayCueParameters Params;
		Params.Instigator = GetAvatarActorFromActorInfo();
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(
			GetAvatarActorFromActorInfo(),
			FireCueTag,
			EGameplayCueEvent::Executed,
			Params);
	}
	CurrentSpreadAngle = FMath::Min(
		CurrentSpreadAngle + SpreadIncreasePerShot, MaxSpreadAngle);
	
	return true;
}

void UGA_FireBase::FireSingleBullet()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;
	//获取武器信息
	UCombatComponent* CombatComp = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!CombatComp) return;
	AShooterWeapon* Weapon = CombatComp->GetCurrentWeapon();
	if (!Weapon) return;

	FVector TargetLocation = CombatComp->GetWeaponTargetLocation();
	FTransform MuzzleTransform = Weapon->CalculateProjectileSpawnTransform(TargetLocation);
	FVector Origin = MuzzleTransform.GetLocation();
	FVector BaseDirection = (TargetLocation - Origin).GetSafeNormal();

	//应用散布
	FVector FinalDirection = ApplySpread(BaseDirection);
	FGameplayEffectSpecHandle DamageSpec;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && BulletConfig->DamageEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddInstigator(AvatarActor, AvatarActor);
		DamageSpec = ASC->MakeOutgoingSpec(
			BulletConfig->DamageEffectClass, GetAbilityLevel(), Context);
	}
	
	if (UBulletManagerComponent* BulletMgr = AvatarActor->FindComponentByClass<UBulletManagerComponent>())
	{
		BulletMgr->FireBullet(BulletConfig, Origin, FinalDirection, DamageSpec);
	}
}

void UGA_FireBase::InitLocalAmmoCount()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return;

	// 2. 从该 AvatarActor 上寻找 CombatComponent
	UCombatComponent* Combat = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!Combat) return;

	AShooterWeapon* Weapon = Combat->GetCurrentWeapon();
	if (!Weapon) return;

	LocalAmmoCount = Weapon->CurrentClipAmmo;
}

bool UGA_FireBase::TryConsumeLocalAmmo()
{
	if (LocalAmmoCount <= 0) return false;

	// 1. 获取释放技能的实体表现 (Avatar Actor)
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor) return false;

	// 本地预扣除
	LocalAmmoCount--;
	
	UCombatComponent* Combat = AvatarActor->FindComponentByClass<UCombatComponent>();
	if (!Combat)return false;
	
	if (AvatarActor->HasAuthority())
	{
		int32 Consumed = Combat->ConsumeClipAmmo(1);
		if (Consumed <= 0) 
		{
			LocalAmmoCount++; 
			return false;
		}
	}else
	{
		Combat->Server_ConsumeClipAmmo(1);
	}
    
	return true;
}

FVector UGA_FireBase::ApplySpread(const FVector& BaseDirection) const
{
	if (CurrentSpreadAngle <= 0.f)
	{
		return BaseDirection;
	}
	// 在锥体内随机偏移
	const float HalfAngleRad = FMath::DegreesToRadians(CurrentSpreadAngle * 0.5f);
	// 均匀分布在锥体内
	const float RandomAngle = FMath::FRandRange(0.f, 2.f * PI);
	// 使用sqrt使分布均匀
	const float RandomRadius = FMath::Sqrt(FMath::FRand()) * FMath::Tan(HalfAngleRad);
	// 构建正交基
	FVector Right, Up;
	BaseDirection.FindBestAxisVectors(Right, Up);
	return (BaseDirection +
		Right * (FMath::Cos(RandomAngle) * RandomRadius) +
		Up * (FMath::Sin(RandomAngle) * RandomRadius)).GetSafeNormal();
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

