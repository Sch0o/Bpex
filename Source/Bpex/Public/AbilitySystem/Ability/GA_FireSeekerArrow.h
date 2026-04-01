// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_FireSeekerArrow.generated.h"

class ASeekerArrow;
/**
 * 
 */
UCLASS()
class BPEX_API UGA_FireSeekerArrow : public UBpexGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_FireSeekerArrow();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Projectile")
	TSubclassOf<ASeekerArrow>ArrowClass;
	
};
