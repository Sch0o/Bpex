// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystem/Ability/CooldownListener.h"
#include "AbilitySlotManagerViewModel.generated.h"

class UAbilitySlotViewModel;
struct FGameplayTag;


USTRUCT(BlueprintType)
struct FAbilitySlotMapping
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CooldownTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySlotViewModel> SlotViewModel = nullptr;
};

UCLASS(BlueprintType)
class BPEX_API UAbilitySlotManagerViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void Initialize(
		UAbilitySystemComponent* InASC,
		const TArray<FAbilitySlotMapping>& InSlotMappings,
		bool bUseServerCooldown);

private:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UAbilitySlotViewModel>> TagToSlotMap;

	UPROPERTY()
	TObjectPtr<UCooldownListener> CooldownListener;

	void HandleCooldownBegin(FGameplayTag Tag, float TimeRemaining, float Duration);

	void HandleCooldownEnd(FGameplayTag Tag, float TimeRemaining, float Duration);
};
