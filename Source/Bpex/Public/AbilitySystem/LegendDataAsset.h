// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LegendTypes.h"
#include "LegendDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API ULegendDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FAbilitySlotInfo TacticalAbility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FAbilitySlotInfo UltimateAbility;
};
