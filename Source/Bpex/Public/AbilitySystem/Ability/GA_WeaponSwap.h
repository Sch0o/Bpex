// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_WeaponSwap.generated.h"

class UCombatComponent;
/**
 * 
 */
UCLASS()
class BPEX_API UGA_WeaponSwap : public UBpexGameplayAbility
{
	GENERATED_BODY()

public:
	
	static constexpr int32 HOLSTER_SLOT = -1;
	
	UGA_WeaponSwap();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr,
	                                FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	//要切换到的目标槽位
	UPROPERTY(BlueprintReadWrite, Category= "Weapon")
	int32 TargetSlotIndex = INDEX_NONE;

private:
	UCombatComponent* GetCombatComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
};
