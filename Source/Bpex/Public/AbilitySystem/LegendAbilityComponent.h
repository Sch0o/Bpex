// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "LegendTypes.h"
#include "LegendAbilityComponent.generated.h"
class UBpexAbilitySystemComponent;
class ULegendDataAsset;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySlotChanged, EAbilitySlotType, SlotType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerkSelected, EShieldTier, Tier, FShieldPerkInfo, Perk);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldTierChanged, EShieldTier, NewTier);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerkSelectionRequired);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API ULegendAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULegendAbilityComponent();

	void InitAbilitySystem(UAbilitySystemComponent* InASC);
	
public:
	UFUNCTION()
	void InitLegendData();
	
private:
	
	UPROPERTY()
	UAbilitySystemComponent*ASC;
	
	UPROPERTY(EditDefaultsOnly, Category = "Abilitt Data")
	TObjectPtr<ULegendDataAsset> LegendData;
	
	UPROPERTY(Transient)
	TMap<EAbilitySlotType, FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	
};
