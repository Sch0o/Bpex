// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "Components/SlateWrapperTypes.h"
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
	bool bIsOnCooldown = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FLinearColor IconTint = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	float CooldownFraction = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	ESlateVisibility CooldownVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify)
	FText CooldownText;

	void SetCooldownTag(FGameplayTag Tag);

	void StartCooldown( float InTimeRemaining ,float InDuration);

	void EndCooldown();

	void SetWorldContext(UWorld* World);

private:
	FGameplayTag CooldownTag;
	
	TWeakObjectPtr<UWorld> CachedWorld;

	float TimeRemaining = 0.f;

	float TotalDuration = 0.f;

	float TickInterval = 0.1f;

	FTimerHandle TimerHandle;

	void TickCooldown();

	void StopTimer();
};
