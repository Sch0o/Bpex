// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API UInvHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void HidePickupMessage();
	void ShowPickupMessage();
	
};
