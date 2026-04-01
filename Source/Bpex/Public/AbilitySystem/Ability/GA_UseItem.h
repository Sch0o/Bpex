// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_UseItem.generated.h"

class UItemUseContext;
class UInventoryItemData;
/**
 * 
 */
UCLASS()
class BPEX_API UGA_UseItem : public UBpexGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY()
	UInventoryItemData* CurrentUseItemData;

	int32 CurrentSlotIndex;
	
	bool bUseSucceeded = false;

	UFUNCTION()
	void OnUseFinished();
	
};
