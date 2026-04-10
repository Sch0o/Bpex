// Fill out your copyright notice in the Description page of Project Settings.


#include "BpexGameplayTags.h"
#include "GameplayTagsManager.h"

void FBpexGameplayTags::InitializeNativeGameplayTags()
{
	Get().Attributes_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.Health"), FString("Health"));

	Get().Attributes_Vital_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Vital.MaxHealth"), FString("MaxHealth"));


	/*
	 * Weapon Tags
	 */
	Get().Cooldown_Weapon_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Cooldown.Weapon.Fire"), FString("Cooldown Tag for Weapon Fire"));

	/*
	* State Tags
	*/
	Get().State_UI_Inventory_Open = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.UI.Inventory.Open"), FString("State for Inventory Open"));
	
	Get().State_UI_UsingItem_Medkit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.UI.UsingItem.Medkit"),FString("State for Using Medkit"));
	
	Get().State_UI_UsingItem_Syringe = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.UI.UsingItem.Syringe"),FString("State for Using Syringe"));
	
	Get().State_Debuff_Revealed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("State.Debuff.Revealed"));

	/*
	* Event Tags
	*/
	Get().Event_UseItem_Medkit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.UseItem.Medkit"),FString("Event for Using Medkit"));
	
	Get().Event_UseItem_Syringe = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Event.UseItem.Syringe"),FString("Event for Using Syringe"));
	
	/*
	* Event Tags
	*/
	Get().Ability_Item_Medkit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Item.Medkit"),FString("Ability for Using Medkit"));
	Get().Ability_Item_Syringe = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Item.Syringe"),FString("Ability for Using Syringe"));
	
	Get().Data_Ammo_DropAmount = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Data.Ammo_DropAmount"),FString("Data Ammo DropAmount"));
	
	/*
	* GameplayCue Tags
	*/
	Get().GameplayCue_Weapon_Rifle_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("GameplayCue.Weapon.Rifle.Fire"),FString("GameplayCure for Weapon Rifle Fire"));
	
	/*
	 * Input Tags
	 */
	Get().InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"), FString("Input Tag for Left Mouse"));

	Get().InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"), FString("Input Tag for Right Mouse"));

	Get().InputTag_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.1"), FString("Input Tag for 1 key"));

	Get().InputTag_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.2"), FString("Input Tag for 2 key"));

	Get().InputTag_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.3"), FString("Input Tag for 3 key"));

	Get().InputTag_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.4"), FString("Input Tag for 4 key"));
	
	
	Get().Item_UseDuration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.UseDuration"),FString("Item UseDuration"));
	
	Get().Item_UseDuration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Item.SlotIndex"),FString("Item SlotIndex"));
	
}

FBpexGameplayTags::FBpexGameplayTags()
{
}

FBpexGameplayTags::~FBpexGameplayTags()
{
}
