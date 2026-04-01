// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BpexHUD.generated.h"

class UAbilitySystemComponent;
struct FItemUIInfo;
class UUserWidget;

UCLASS()
	class BPEX_API ABpexHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> MainHUDClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI|Shooter")
	TObjectPtr<UUserWidget> MainHUDWidget;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Pickup")
	void ShowPickupMessage(const FItemUIInfo& Message);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Pickup")
	void HidePickupMessage();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS|UI")
	void BP_OnAbilitySystemInitialized(UAbilitySystemComponent* TargetASC);
	
	
};
