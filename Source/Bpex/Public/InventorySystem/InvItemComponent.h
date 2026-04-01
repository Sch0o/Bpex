// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InvItemComponent.generated.h"

USTRUCT(BlueprintType)
struct FItemUIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Info")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Info")
	FText PickupMessage;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UInvItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	const FItemUIInfo& GetItemInfo() const { return ItemInfo; }

protected:
	UInvItemComponent();
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FItemUIInfo ItemInfo;
};
