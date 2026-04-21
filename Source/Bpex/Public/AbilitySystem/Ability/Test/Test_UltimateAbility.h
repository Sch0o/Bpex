// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/LegendGameplayAbility.h"
#include "Test_UltimateAbility.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UTest_UltimateAbility : public ULegendUltimateAbility
{
	GENERATED_BODY()
public:
	UTest_UltimateAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							 const FGameplayAbilityActivationInfo ActivationInfo,
							 const FGameplayEventData* TriggerEventData) override;
private:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Test",meta = (AllowPrivateAccess = true))
	TSubclassOf<class UGameplayEffect> HealEffectClass;
};
