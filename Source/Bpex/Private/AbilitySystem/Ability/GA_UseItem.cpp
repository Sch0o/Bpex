// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_UseItem.h"
#include "InventorySystem/Types/InventoryTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "InventorySystem/InventoryComponent.h"

void UGA_UseItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                  const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::ActivateAbility"))
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData)
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::TriggerEventData is invalid"))
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CurrentSlotIndex = FMath::RoundToInt(TriggerEventData->EventMagnitude);
	
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;
	UInventoryComponent* Inventory = Avatar->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::OnUseFinished():Inventory is null"));
	}
	
	CurrentUseItemData=Inventory->GetInventoryItemData(CurrentSlotIndex);
	if (!IsValid(CurrentUseItemData))
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::CurrentUseItemData is invalid"))
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	float Duration = CurrentUseItemData->UseDuration;
	UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	Task->OnFinish.AddDynamic(this, &UGA_UseItem::OnUseFinished);
	Task->ReadyForActivation();
}

void UGA_UseItem::OnUseFinished()
{
	UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::OnUseFinished"))
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UInventoryComponent* Inventory = Avatar->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); 
		return;
	}
	
	UInventoryItemData* ItemAtFinish = Inventory->GetInventoryItemData(CurrentSlotIndex);
	if (ItemAtFinish != CurrentUseItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("item was moved"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true); // 按被打断处理
		return;
	}

	if (CurrentUseItemData->ItemEffect)
	{
		UE_LOG(LogTemp,Warning,TEXT("UGA_UseItem::apply effect"))
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
			CurrentUseItemData->ItemEffect, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
		}
	}
	
	if (GetActorInfo().IsNetAuthority())
	{
		Inventory->ConsumeItemInSlot(CurrentSlotIndex, 1);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
