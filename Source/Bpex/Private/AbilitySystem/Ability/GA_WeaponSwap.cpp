// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_WeaponSwap.h"

#include "BpexGameplayTags.h"
#include "Weapon/CombatComponent.h"

UGA_WeaponSwap::UGA_WeaponSwap()
{
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FBpexGameplayTags::Get().Event_Weapon_Swap;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
}

void UGA_WeaponSwap::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UCombatComponent* CombatComp = GetCombatComponent(ActorInfo);
	if (!CombatComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//从事件数据中读取目标槽位
	if (TriggerEventData)
	{
		TargetSlotIndex = FMath::TruncToInt(TriggerEventData->EventMagnitude);
	}
	
	if (TargetSlotIndex == HOLSTER_SLOT)
	{
		// 已经是空手状态，不重复执行
		if (!CombatComp->HasWeaponEquipped())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		
		CombatComp->Holster();
		TargetSlotIndex = INDEX_NONE;
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	
	if (TargetSlotIndex == INDEX_NONE)
	{
		// 如果没指定槽位，切换到下一个有效槽位
		int32 CurrentSlot = CombatComp->GetActiveSlotIndex();
		int32 NumSlots = CombatComp->GetNumSlots();
        
		for (int32 i = 1; i < NumSlots; ++i)
		{
			int32 NextSlot = (CurrentSlot + i) % NumSlots;
			if (CombatComp->IsSlotValid(NextSlot))
			{
				TargetSlotIndex = NextSlot;
				break;
			}
		}
	}
	
	//验证目标槽位，目标槽位不能是当前槽位
	if (TargetSlotIndex == INDEX_NONE ||
		!CombatComp->IsSlotValid(TargetSlotIndex) ||TargetSlotIndex == CombatComp->GetActiveSlotIndex())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CombatComp->EquipSlotWeapon(TargetSlotIndex);
	
	TargetSlotIndex = INDEX_NONE;

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_WeaponSwap::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	UCombatComponent *CombatComponent = GetCombatComponent(ActorInfo);
	if (!CombatComponent)
	{
		return false;
	}
	return true;
}


UCombatComponent* UGA_WeaponSwap::GetCombatComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return nullptr;
    
	return ActorInfo->AvatarActor->FindComponentByClass<UCombatComponent>();
}