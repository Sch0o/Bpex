// Fill out your copyright notice in the Description page of Project Settings.


#include "BpexAssetManager.h"
#include "BpexGameplayTags.h"

UBpexAssetManager& UBpexAssetManager::Get()
{
	check(GEngine)
	UBpexAssetManager* BpexAssetManager = Cast<UBpexAssetManager>(GEngine->AssetManager);
	return *BpexAssetManager;
}

void UBpexAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FBpexGameplayTags::Get().InitializeNativeGameplayTags();
}
