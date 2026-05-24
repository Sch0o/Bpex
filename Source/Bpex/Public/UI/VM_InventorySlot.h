// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_InventorySlot.generated.h"

class UInventoryItemData;

UCLASS(Blueprintable)
class BPEX_API UVM_InventorySlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category="Inventory|Slot",
		meta = (AllowPrivateAccess="true"))
	UInventoryItemData* ItemData;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category="Inventory|Slot",
		meta = (AllowPrivateAccess="true"))
	UTexture2D* ItemIcon;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category="Inventory|Slot",
		meta = (AllowPrivateAccess="true"))
	int Quantity;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, Category="Inventory|Slot",
		meta = (AllowPrivateAccess="true"))
	int32 OriginalNetIndex;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Getter, Category = "Inventory|Slot", meta = (AllowPrivateAccess="true"))
	bool bHasItem = false;

public:
	UInventoryItemData* GetItemData() const { return ItemData; }
	UTexture2D* GetItemIcon() const { return ItemIcon; }
	int32 GetQuantity() const { return Quantity; }
	int32 GetOriginalNetIndex() const { return OriginalNetIndex; }
	bool GetbHasItem() const { return bHasItem; }

	void SetItemData(UInventoryItemData* NewItemData);
	void SetItemIcon(UTexture2D* NewItemIcon);
	void SetQuantity(int32 NewQuantity);
	void SetOriginalNetIndex(int32 NewIndex);
};
