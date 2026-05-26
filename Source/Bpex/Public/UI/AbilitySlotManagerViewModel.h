// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystem/Ability/CooldownListener.h"
#include "AbilitySlotManagerViewModel.generated.h"

class UAbilitySlotViewModel;
struct FGameplayTag;
/**
 * 
 */
UCLASS(BlueprintType)
class BPEX_API UAbilitySlotManagerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(UAbilitySystemComponent* InASC, const FGameplayTagContainer& InCooldownTags,
	                bool bUseServerCooldown);

	UFUNCTION()
	UAbilitySlotViewModel* GetSlotByTag(FGameplayTag Tag) const;

private:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UAbilitySlotViewModel>> TagToSlotMap;

	UPROPERTY()
	TObjectPtr<UCooldownListener> CooldownLister;

	void HandleCooldownBegin(FGameplayTag Tag, float TimeRemaining, float Duration);

	void HandleCooldownEnd(FGameplayTag Tag, float TimeRemaining, float Duration);
};
