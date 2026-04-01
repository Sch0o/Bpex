// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_FireBase.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UGA_FireBase : public UBpexGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_FireBase();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta=(DisplayName="On Input Released"))
	void K2_OnInputReleased();

protected:
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon|Hitscan")
	float TimeBetweenShots = 0.5f;
	
	FTimerHandle AutoFireTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Cues")
	FGameplayTag FireCueTag;
	
	//单次开火逻辑
	UFUNCTION(BlueprintCallable,Category="Weapon")
	void AutoFireTick();
	
	UFUNCTION(BlueprintImplementableEvent,Category="Ability",meta=(DisplayName="On Auto Fire Tick"))
	void K2_OnAutoFireTick();
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon|Hitscan")
	float HitscanRange = 10000.f;
	
	UFUNCTION(BlueprintCallable,Category="Weapon")
	void PerformHitscan();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<class AShooterProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere,Category="Damage")
	TSubclassOf<UGameplayEffect>DamageEffectClass;

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SpawnProjectile();
	
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
};
