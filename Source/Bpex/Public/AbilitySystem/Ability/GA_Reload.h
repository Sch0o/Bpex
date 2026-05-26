// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_Reload.generated.h"

class UInventoryComponent;
class UCombatComponent;
/**
 * 
 */
UCLASS()
class BPEX_API UGA_Reload : public UBpexGameplayAbility
{
	GENERATED_BODY()

	UGA_Reload();
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable,Category="Weapon")
	void PerformReload();

private:
	UPROPERTY()
	TObjectPtr<UCombatComponent> CachedCombatComp;
	UPROPERTY()
	TObjectPtr<UInventoryComponent> CachedInventoryComp;
};
