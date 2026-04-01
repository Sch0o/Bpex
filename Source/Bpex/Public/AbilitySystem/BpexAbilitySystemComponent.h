// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BpexAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEffectAssetTagsAppliedSignature, const FGameplayTagContainer&)

UCLASS()
class BPEX_API UBpexAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

protected :
	void EffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec,
	                   FActiveGameplayEffectHandle ActiveEffectHandle);

public:
	void AbilityActorInfoSet();
	
	void AbilityInputHeld(const FGameplayTag&InputTag);
	
	void AbilityInputPressed(const FGameplayTag&InputTag);
	
	void AbilityInputReleased(const FGameplayTag&InputTag);

	FOnEffectAssetTagsAppliedSignature OnEffectAssetTagsApplied;
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>&Abilities);
	
	void DebugPrintTriggerMapping();
};
