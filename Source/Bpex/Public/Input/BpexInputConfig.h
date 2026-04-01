// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BpexInputConfig.generated.h"

USTRUCT(Blueprintable)
struct FBpexInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

UCLASS()
class BPEX_API UBpexInputConfig : public UDataAsset
{
	GENERATED_BODY()
public:
	
	const UInputAction* FindAbilityInputActionByTag(const FGameplayTag &InputTag,bool bLogNotFound) const;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	TArray<FBpexInputAction> AbilityInputActions;
};
