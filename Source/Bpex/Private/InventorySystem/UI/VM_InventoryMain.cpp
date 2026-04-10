// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/UI/VM_InventoryMain.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/Types/InventoryTypes.h"
#include "InventorySystem/UI/VM_InventorySlot.h"

UVM_InventoryMain::UVM_InventoryMain()
{
	TotalSlots = 0;
}

void UVM_InventoryMain::InitializeViewModels(UInventoryComponent* InventoryComp)
{
	UE_LOG(LogTemp,Warning,TEXT("UVM_InventoryMain::InitializeViewModels"));
	if (!InventoryComp) return;
	TotalSlots = InventoryComp->Items().Num();

	SlotViewModels.Empty();
	for (int i = 0; i < TotalSlots; i++)
	{
		UVM_InventorySlot* NewSlotVM = NewObject<UVM_InventorySlot>(this);
		NewSlotVM->SetItemIcon(nullptr);
		NewSlotVM->SetQuantity(0);
		SlotViewModels.Add(NewSlotVM);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SlotViewModels);
}

void UVM_InventoryMain::RefreshInventoryData(UInventoryComponent* InventoryComp)
{
	UE_LOG(LogTemp,Warning,TEXT("UVM_InventoryMain::RefreshInventoryData"));
	if (!InventoryComp || SlotViewModels.Num() == 0) return;
	const TArray<FInventorySlot>& InventorySlotArray = InventoryComp->Items();
	for (int32 i = 0; i < InventorySlotArray.Num(); i++)
	{
		UVM_InventorySlot* CurrentSlotVM = SlotViewModels[i];
		CurrentSlotVM->SetOriginalNetIndex(i);
		if (InventorySlotArray.IsValidIndex(i)&&InventorySlotArray[i].ItemInfo!=nullptr)
		{
			CurrentSlotVM->SetItemData(InventorySlotArray[i].ItemInfo);
			CurrentSlotVM->SetQuantity(InventorySlotArray[i].Quantity);
		}else
		{
			CurrentSlotVM->SetItemData(nullptr);
			CurrentSlotVM->SetQuantity(0);
		}
	}
	
	SlotViewModels.Sort([](const UVM_InventorySlot& A, const UVM_InventorySlot& B)
	{
		bool bHasItemA = A.GetItemData() != nullptr;
		bool bHasItemB = B.GetItemData() != nullptr;

		if (bHasItemA && !bHasItemB) return true;
		if (!bHasItemA && bHasItemB) return false;

		if (!bHasItemA && !bHasItemB)
		{
			return A.GetOriginalNetIndex() < B.GetOriginalNetIndex();
		}

		if (A.GetItemData()->SortPriority != B.GetItemData()->SortPriority)
		{
			return A.GetItemData()->SortPriority > B.GetItemData()->SortPriority;
		}
		
		return A.GetQuantity() > B.GetQuantity();
	});
	
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SlotViewModels);
}
