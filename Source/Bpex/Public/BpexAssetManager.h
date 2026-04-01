// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "BpexAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UBpexAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UBpexAssetManager& Get();

protected:
	
	virtual void StartInitialLoading() override;
};
