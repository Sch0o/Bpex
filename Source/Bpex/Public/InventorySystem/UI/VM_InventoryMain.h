// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_InventoryMain.generated.h"

struct FGameplayTag;
class UAbilitySystemComponent;
class UVM_InventorySlot;
class UInventoryComponent;

UCLASS(Blueprintable)
class BPEX_API UVM_InventoryMain : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	UVM_InventoryMain();
	
	//所有格子的viewmodel
	UPROPERTY(BlueprintReadOnly,FieldNotify,Category="Inventory")
	TArray<UVM_InventorySlot*> SlotViewModels;
	
	//当前背包的槽位数量
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="Inventory")
	int32 TotalSlots;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void InitializeViewModels(UInventoryComponent*InventoryComp);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void RefreshInventoryData(UInventoryComponent*InventoryComp);
	
	
};
