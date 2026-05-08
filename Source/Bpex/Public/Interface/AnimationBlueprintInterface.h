// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BpexTypes.h"
#include "AnimationBlueprintInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UAnimationBlueprintInterface : public UInterface
{
	GENERATED_BODY()
};
class BPEX_API IAnimationBlueprintInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category="Animation")
	void ReceiveGroundDistance(float GroundDistance);
	
	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable,Category="Animation")
	void ReceiveCurrentGate(EGate CurrentGate);
};
