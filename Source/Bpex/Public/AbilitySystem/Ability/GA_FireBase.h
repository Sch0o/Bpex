// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexGameplayAbility.h"
#include "GA_FireBase.generated.h"

class UBulletDataAsset;
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

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;


	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta=(DisplayName="On Input Released"))
	void K2_OnInputReleased();

protected:
	//子弹配置
	int32 LocalAmmoCount = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	UBulletDataAsset* BulletConfig = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	float TimeBetweenShots = 0.075f;

	//散布
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float CurrentSpreadAngle = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float SpreadIncreasePerShot = 0.3f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float MaxSpreadAngle = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Spread")
	float SpreadRecoveryRate = 8.f;

	//Cue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Cues")
	FGameplayTag FireCueTag;

	FTimerHandle AutoFireTimerHandle;
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AutoFireTick();
	UFUNCTION(BlueprintImplementableEvent, Category="Ability", meta=(DisplayName="On Auto Fire Tick"))
	void K2_OnAutoFireTick();
	void FireSingleBullet();
	void InitLocalAmmoCount();
	bool TryConsumeLocalAmmo();

	FVector ApplySpread(const FVector& BaseDirection) const;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) override;
};
