// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_Reload.h"
#include "BpexGameplayTags.h"
#include "InventorySystem/InventoryComponent.h"
#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"
#include "AbilitySystemComponent.h"

class AShooterWeapon;

UGA_Reload::UGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	ActivationBlockedTags.AddTag(FBpexGameplayTags::Get().State_Dead);
}

bool UGA_Reload::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor) return false;

	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);

	if (!Spec)
	{
		return false;
	}

	AShooterWeapon* Weapon = Cast<AShooterWeapon>(Spec->SourceObject.Get());
	if (!Weapon || Weapon->IsClipFull())
	{
		return false;
	}
	
	// 检查背包是否有对应子弹
	UInventoryComponent* InventoryComp = AvatarActor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp) return false;

	if (InventoryComp->GetAmmoCount(Weapon->AmmoType) <= 0) return false;
	return true;
}

void UGA_Reload::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	CachedCombatComp = AvatarActor->FindComponentByClass<UCombatComponent>();
	CachedInventoryComp = AvatarActor->FindComponentByClass<UInventoryComponent>();
	if (!CachedCombatComp || !CachedInventoryComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UGA_Reload::PerformReload()
{
	if (!CachedCombatComp || !CachedInventoryComp) return;
	AShooterWeapon* Weapon = CachedCombatComp->GetCurrentWeapon();
	if (!Weapon) return;
	// 需要多少子弹才能填满弹匣
	int32 AmmoNeeded = Weapon->MaxClipAmmo - Weapon->CurrentClipAmmo;
	if (AmmoNeeded <= 0) return;
	// 背包有多少对应子弹
	int32 AmmoAvailable = CachedInventoryComp->GetAmmoCount(Weapon->AmmoType);
	int32 AmmoToLoad = FMath::Min(AmmoNeeded, AmmoAvailable);
	if (AmmoToLoad <= 0) return;
	// 从背包扣子弹
	int32 Consumed = CachedInventoryComp->ConsumeAmmo(Weapon->AmmoType, AmmoToLoad);
	// 填入弹匣
	Weapon->AddClipAmmo(Consumed);
	// 刷新UI
	CachedCombatComp->BroadcastAmmoUI();
	UE_LOG(LogTemp, Log, TEXT("GA_Reload: Loaded %d rounds into %s"), Consumed, *Weapon->GetName());
}
