// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_FireBase.h"

#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterProjectile.h"
#include "Weapon/ShooterWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "Weapon/BulletManagerComponent.h"
#include "Weapon/BulletTypes.h"

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
		AutoFireTick(); // 立即开第一枪
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireTimerHandle, this,
			&UGA_FireBase::AutoFireTick, TimeBetweenShots, true);
	}
}

void UGA_FireBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                              const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                              bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_FireBase::AutoFireTick()
{
	if (!TryConsumeLocalAmmo())
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_FireBase::AutoFireTick:: out of ammo"));
		K2_OnInputReleased();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
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

	//累计散布
	CurrentSpreadAngle = FMath::Min(
		CurrentSpreadAngle + SpreadIncreasePerShot, MaxSpreadAngle);
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
	UBulletManagerComponent* BulletMgr = AvatarActor->FindComponentByClass<UBulletManagerComponent>();
	if (BulletMgr)
	{
		BulletMgr->FireBullet(BulletConfig, Origin, FinalDirection, DamageSpec);
	}
}

void UGA_FireBase::InitLocalAmmoCount()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		LocalAmmoCount = ASC->GetNumericAttribute(UBpexAttributeSet::GetClipAmmoAttribute());
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_FireBase::InitLocalAmmoCount::ASC is null"));
	}
}

bool UGA_FireBase::TryConsumeLocalAmmo()
{
	if (LocalAmmoCount <= 0)
	{
		return false;
	}
	LocalAmmoCount--;
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
