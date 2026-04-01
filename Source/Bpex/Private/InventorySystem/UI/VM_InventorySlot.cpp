// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/UI/VM_InventorySlot.h"
#include "InventorySystem/Types/InventoryTypes.h"

void UVM_InventorySlot::SetItemIcon(UTexture2D* NewItemIcon)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemIcon, NewItemIcon);
}

void UVM_InventorySlot::SetQuantity(int32 NewQuantity)
{
	UE_MVVM_SET_PROPERTY_VALUE(Quantity, NewQuantity);
}

void UVM_InventorySlot::SetItemData(UInventoryItemData* NewItemData)
{
	UE_MVVM_SET_PROPERTY_VALUE(ItemData, NewItemData);
	
	// 自动更新图标和状态
	SetItemIcon(NewItemData ? NewItemData->ItemIcon : nullptr);
	UE_MVVM_SET_PROPERTY_VALUE(bHasItem, NewItemData != nullptr);
}
void UVM_InventorySlot::SetOriginalNetIndex(int32 NewIndex)
{
	UE_MVVM_SET_PROPERTY_VALUE(OriginalNetIndex, NewIndex);
}
