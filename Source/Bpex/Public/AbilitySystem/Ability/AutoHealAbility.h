// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LegendGameplayAbility.h"
#include "AutoHealAbility.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UAutoHealAbility : public ULegendGameplayAbility
{
	GENERATED_BODY()
	
public:
	UAutoHealAbility();
	
	virtual void ActivateAbility(
	  const FGameplayAbilitySpecHandle Handle,
	  const FGameplayAbilityActorInfo* ActorInfo,
	  const FGameplayAbilityActivationInfo ActivationInfo,
	  const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AutoHeal")
	FGameplayTag DataTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AutoHeal")
	TSubclassOf<UGameplayEffect> HealEffectClass;
	
};
