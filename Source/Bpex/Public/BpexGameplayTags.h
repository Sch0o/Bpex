// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"


class BPEX_API FBpexGameplayTags
{
public:
	static FBpexGameplayTags& Get()
	{
		static FBpexGameplayTags instance;
		return instance;
	}

	static void InitializeNativeGameplayTags();
	FBpexGameplayTags(const FBpexGameplayTags&) = delete;
	FBpexGameplayTags& operator=(const FBpexGameplayTags&) = delete;

	FGameplayTag Attributes_Vital_Health;
	FGameplayTag Attributes_Vital_MaxHealth;
	
	FGameplayTag State_UI_Inventory_Open;
	FGameplayTag State_UI_UsingItem_Medkit;
	FGameplayTag State_UI_UsingItem_Syringe;
	
	FGameplayTag State_Debuff_Revealed;
	FGameplayTag State_Armed;
	FGameplayTag State_UnArmed;
	FGameplayTag State_Action_Firing;
	FGameplayTag State_Action_Swapping;
	FGameplayTag State_Dead;
	
	FGameplayTag Event_UseItem_Medkit;
	FGameplayTag Event_UseItem_Syringe;
	FGameplayTag Event_Weapon_Swap;
	
	FGameplayTag Ability_Item_Syringe;
	FGameplayTag Ability_Item_Medkit;
	FGameplayTag Ability_Weapon_NoFiring;
	
	FGameplayTag GameplayCue_Weapon_Rifle_Fire;
	
	FGameplayTag Item_UseDuration;
	FGameplayTag Item_SlotIndex;
	
	FGameplayTag Data_Ammo_DropAmount;
	
	
	FGameplayTag Cooldown_Weapon_Fire;
	FGameplayTag Cooldown_Ability_TestTactical;
	FGameplayTag Cooldown_Ability_TestUltimate;
	
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_Weapon_Slot1;
	FGameplayTag InputTag_Weapon_Slot2;
	FGameplayTag InputTag_Weapon_Holster;
	FGameplayTag InputTag_Weapon_Reload;
	FGameplayTag InputTag_Weapon_Fire;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	
	FGameplayTag Gameplay_AbilityInputBlocked;

private:
	FBpexGameplayTags();
	~FBpexGameplayTags();
};
