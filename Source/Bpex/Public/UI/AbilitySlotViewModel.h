// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "AbilitySlotViewModel.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UAbilitySlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FGameplayTag CooldownTag;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float TimeRemaining = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float Duration = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float Fraction = 0.f;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	bool bIsOnCooldown = false;
	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FText CooldownText;
	
	void StartCooldown(UWorld* World, float InTimeRemaining,float InDuration);
private:
	TWeakObjectPtr<UWorld> CachedWorld;
	
	float TickInterval = 0.1f;
	
	FTimerHandle TimerHandle;
	
	void TickCooldown();
	
	void StopTimer();
};
