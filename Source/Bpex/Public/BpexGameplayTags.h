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
	
	FGameplayTag Event_UseItem_Medkit;
	FGameplayTag Event_UseItem_Syringe;
	
	FGameplayTag Ability_Item_Syringe;
	FGameplayTag Ability_Item_Medkit;
	
	FGameplayTag GameplayCue_Weapon_Rifle_Fire;
	
	FGameplayTag Item_UseDuration;
	FGameplayTag Item_SlotIndex;
	
	FGameplayTag Data_Ammo_DropAmount;
	
	/*
	 * weapon
	 */
	FGameplayTag Cooldown_Weapon_Fire;
	
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

private:
	FBpexGameplayTags();
	~FBpexGameplayTags();
};
