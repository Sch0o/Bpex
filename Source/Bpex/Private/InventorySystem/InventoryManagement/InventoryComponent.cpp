// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/Types/InventoryTypes.h"
#include "InventorySystem/Actors/BpexItemActor.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"

// ═════════════════════════════════════════════
//  组件本体
// ═════════════════════════════════════════════

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	MaxCapacity = 10;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	SlotArray.OwnerComponent = this;
	//初始化FastArray
	if (GetOwner()->HasAuthority())
	{
		Items().Init(FInventorySlot(), MaxCapacity);
		SlotArray.MarkAllDirty();
	}
	ConstructInventory();
}

int32 UInventoryComponent::TryAddItem(UInventoryItemData* ItemToAdd, int32& QuantityToAdd)
{
	if (!ItemToAdd || QuantityToAdd <= 0) return 0;

	//一共要堆叠的数量
	int OriginalQuantity = QuantityToAdd;
	//找同类别且未满的堆叠,只会有一个
	for (FInventorySlot& Slot : Items())
	{
		if (Slot.ItemInfo && Slot.ItemInfo->ItemID == ItemToAdd->ItemID && Slot.Quantity < ItemToAdd->MaxStackSize)
		{
			int32 Spaceleft = ItemToAdd->MaxStackSize - Slot.Quantity;
			int32 AmountToStack = FMath::Min(Spaceleft, QuantityToAdd);
			Slot.Quantity += AmountToStack;
			QuantityToAdd -= AmountToStack;
			SlotArray.MarkSlotDirty(Slot);
			//装完了
			if (QuantityToAdd <= 0)
			{
				break;
			}
		}
	}
	//找空格子
	bool HasPut = false;
	if (QuantityToAdd > 0)
	{
		for (FInventorySlot& Slot : Items())
		{
			if (Slot.ItemInfo) continue;
			HasPut = true;
			Slot.ItemInfo = ItemToAdd;
			int32 AmountToStack = FMath::Min(ItemToAdd->MaxStackSize, QuantityToAdd);
			Slot.Quantity = AmountToStack;
			QuantityToAdd -= AmountToStack;
			SlotArray.MarkSlotDirty(Slot);
			if (QuantityToAdd <= 0)
			{
				return OriginalQuantity;
			}
		}
	}
	int32 AmountAdded = OriginalQuantity - QuantityToAdd;
	return AmountAdded;
}

void UInventoryComponent::TryUseItem(int32 SlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::TryUseItem"));
	if (!Items().IsValidIndex(SlotIndex) || Items()[SlotIndex].Quantity <= 0) return;

	UInventoryItemData* ItemData = Items()[SlotIndex].ItemInfo;
	if (!ItemData || !ItemData->UseEventTag.IsValid()) return;


	GetLastSameItemSlotIndex(SlotIndex, ItemData);
	AActor* OwnerActor = GetOwner();
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!ASI) return;

	if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
	{
		//通知viewmodel技能的持续时间
		OnItemUseStarted.Broadcast(ItemData->UseDuration);

		FGameplayEventData Payload;
		Payload.Instigator = GetOwner();
		Payload.Target = GetOwner();
		Payload.EventMagnitude = static_cast<float>(SlotIndex);
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::TryUseItem::HandleGameplayEvent%s"),
		       *ItemData->UseEventTag.ToString());
		FScopedPredictionWindow NewScopedWindow(ASC, true);
		int32 Triggered = ASC->HandleGameplayEvent(ItemData->UseEventTag, &Payload);
		UE_LOG(LogTemp, Warning, TEXT("HandleGameplayEvent triggered %d abilities"), Triggered);
	}
}

void UInventoryComponent::GetLastSameItemSlotIndex(int32& SlotIndex, UInventoryItemData* ItemData)
{
	for (int32 i = Items().Num() - 1; i >= 0; i--)
	{
		if (Items()[i].ItemInfo == ItemData)
		{
			SlotIndex = i;
			break;
		}
	}
}

void UInventoryComponent::TryDropItem(int32 SlotIndex)
{
	//RPC请求
	if (!GetOwner()->HasAuthority())
	{
		Server_RequestDropItem(SlotIndex);
		return;
	}

	TArray<FInventorySlot>& InventorySlots = Items();

	if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].ItemInfo == nullptr)
	{
		return;
	}

	UInventoryItemData* ItemData = InventorySlots[SlotIndex].ItemInfo;

	GetLastSameItemSlotIndex(SlotIndex, ItemData);

	if (!ItemData->ItemClass) return;

	AActor* OwnerActor = GetOwner();
	if (OwnerActor && GetWorld())
	{
		FVector SpawnLocation = OwnerActor->GetActorLocation() + (OwnerActor->GetActorForwardVector() * 50.0f);
		SpawnLocation.Z += 50.0f;
		FRotator SpawnRotation = OwnerActor->GetActorRotation();

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ItemData->ItemClass, SpawnLocation, SpawnRotation);

		if (SpawnedActor)
		{
			ABpexItemActor* DroppedItem = Cast<ABpexItemActor>(SpawnedActor);
			DroppedItem->SetQuantity(FMath::Min(ItemData->DefaultDropAmount,InventorySlots[SlotIndex].Quantity));

			UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent());

			if (PrimitiveComp)
			{
				PrimitiveComp->SetSimulatePhysics(true);

				FVector ThrowDirection = OwnerActor->GetActorForwardVector() + FVector(0.0f, 0.0f, 0.5f);
				ThrowDirection.Normalize();

				FVector Impulse = ThrowDirection * ThrowForce;

				// 施加冲量。第三个参数 bVelocityChange 设为 true 代表无视物体质量（直接改变速度），这样不同重量的物品扔出的距离是一样的。
				PrimitiveComp->AddImpulse(Impulse, NAME_None, true);
			}
		}

		FInventorySlot& ModifiedSlot = InventorySlots[SlotIndex];
		int32 DropAmount = FMath::Min(ItemData->DefaultDropAmount,ModifiedSlot.Quantity);
		ModifiedSlot.Quantity -= DropAmount;
		if (ModifiedSlot.Quantity <= 0)
		{
			ModifiedSlot.Quantity = 0;
			ModifiedSlot.ItemInfo = nullptr;
		}
		SlotArray.MarkSlotDirty(ModifiedSlot);

		if (ItemData->ModifyAttributeEffect)
		{
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			if (ASC)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
					ItemData->ModifyAttributeEffect, 1.0f, Context);

				if (SpecHandle.IsValid())
				{
					SpecHandle.Data.Get()->SetSetByCallerMagnitude(ItemData->MagnitudeTag,
					                                               static_cast<float>(-DropAmount));
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
}

void UInventoryComponent::ConsumeItemInSlot(int32 SlotIndex, int32 ConsumeQuantity)
{
	if (!Items().IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ConsumeItem: 无效的槽位索引 %d"), SlotIndex);
		return;
	}
	FInventorySlot& SlotToModify = Items()[SlotIndex];
	if (SlotToModify.ItemInfo == nullptr || SlotToModify.Quantity <= 0) return;
	SlotToModify.Quantity -= ConsumeQuantity;
	if (SlotToModify.Quantity <= 0)
	{
		SlotToModify.Quantity = 0;
		SlotToModify.ItemInfo = nullptr;
	}
	SlotArray.MarkSlotDirty(SlotToModify);
}

void UInventoryComponent::ExpandInventory(int32 AdditionalSlots)
{
	if (AdditionalSlots > 0)
	{
		MaxCapacity = AdditionalSlots;
		Items().AddDefaulted(AdditionalSlots);
	}
}

// ── Server RPCs ──

void UInventoryComponent::Server_RequestDropItem_Implementation(int32 SlotIndex)
{
	TryDropItem(SlotIndex);
}

bool UInventoryComponent::Server_RequestDropItem_Validate(int32 SlotIndex)
{
	if (!Items().IsValidIndex(SlotIndex) || Items()[SlotIndex].ItemInfo == nullptr)
	{
		return false;
	}
	return true;
}

void UInventoryComponent::Server_RequestUseItem_Implementation(int32 SlotIndex)
{
	TryUseItem(SlotIndex);
}

bool UInventoryComponent::Server_RequestUseItem_Validate(int32 SlotIndex)
{
	if (!Items().IsValidIndex(SlotIndex) || Items()[SlotIndex].ItemInfo == nullptr)
	{
		return false;
	}
	return true;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//只同步给拥有背包的玩家
	DOREPLIFETIME_CONDITION(UInventoryComponent, SlotArray, COND_OwnerOnly);
}

bool UInventoryComponent::Server_RequestPickUpItem_Validate(ABpexItemActor* ItemToPickUp)
{
	//传入物品为空，或者已经销毁
	if (!ItemToPickUp) return false;
	return true;
}

void UInventoryComponent::Server_RequestPickUpItem_Implementation(ABpexItemActor* ItemToPickUp)
{
	UInventoryItemData* ItemData = ItemToPickUp->GetItemData();
	if (ItemData == nullptr)
	{
		UE_LOG(LogTemp,Warning,TEXT("UInventoryComponent::Server_RequestPickUpItem_Implementation::ItemData is null"));
	}
	//地上有多少
	int32 QuantityOnGround = ItemToPickUp->GetQuantity();

	//尝试放入背包
	int32 OriginalQuantity = QuantityOnGround;
	TryAddItem(ItemData, QuantityOnGround);

	int32 AmountAdded = OriginalQuantity - QuantityOnGround;
	if (AmountAdded > 0 && ItemData->ModifyAttributeEffect)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (ASC)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddInstigator(GetOwner(), GetOwner());
			FGameplayEffectSpecHandle SpecHandle = ASC->
				MakeOutgoingSpec(ItemData->ModifyAttributeEffect, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				// 传入增加的数量 (正数)
				SpecHandle.Data.Get()->SetSetByCallerMagnitude(ItemData->MagnitudeTag, static_cast<float>(AmountAdded));
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	//全部销毁
	if (QuantityOnGround <= 0)
	{
		ItemToPickUp->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("Item [%s] fully picked up. Remaining: %d"), *ItemToPickUp->GetName(),
		       QuantityOnGround)
	}
	else if (OriginalQuantity > QuantityOnGround)
	{
		ItemToPickUp->SetQuantity(QuantityOnGround);
		UE_LOG(LogTemp, Warning, TEXT("Item [%s] partially picked up. Remaining: %d"), *ItemToPickUp->GetName(),
		       QuantityOnGround);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Failed to pick up [%s]. Inventory might be full."), *ItemToPickUp->GetName());
	}
}

// ── UI ──

void UInventoryComponent::InteractInventory()
{
	if (!InventoryMenu)
	{
		UE_LOG(LogTemp, Warning, TEXT("have no InventoryMenu"));
		return;
	}
	if (!OwningController.IsValid()) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!ASI) return;
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	if (bIsShowingInventoryMenu)
	{
		InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);

		if (InventoryOpenTag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(InventoryOpenTag);
		}

		OwningController->bShowMouseCursor = false;

		FInputModeGameOnly InputMode;
		OwningController->SetInputMode(InputMode);
	}
	else
	{
		InventoryMenu->SetVisibility(ESlateVisibility::Visible);

		if (InventoryOpenTag.IsValid())
		{
			ASC->AddLooseGameplayTag(InventoryOpenTag);
		}

		OwningController->bShowMouseCursor = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryMenu->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningController->SetInputMode(InputMode);
	}
	bIsShowingInventoryMenu = !bIsShowingInventoryMenu;
}

void UInventoryComponent::CloseInventory()
{
	if (bIsShowingInventoryMenu && InventoryMenu)
	{
		InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
		if (OwningController.IsValid())
		{
			OwningController->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			OwningController->SetInputMode(InputMode);
		}

		bIsShowingInventoryMenu = false;
	}

	IAbilitySystemInterface* IAS = Cast<IAbilitySystemInterface>(GetOwner());
	if (!IAS) return;
	UAbilitySystemComponent* ASC = IAS->GetAbilitySystemComponent();
	if (!ASC) return;
	if (InventoryOpenTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(InventoryOpenTag);
	}
}

void UInventoryComponent::OnSlotUpdated()
{
	OnInventoryUpdated.Broadcast();
}

// ═════════════════════════════════════════════
//  Helper
// ═════════════════════════════════════════════

float UInventoryComponent::GetInventoryItemUseDuration(int32 SlotIndex) const
{
	if (Items().IsValidIndex(SlotIndex))
	{
		return Items()[SlotIndex].ItemInfo->UseDuration;
	}
	return 0;
}

UInventoryItemData* UInventoryComponent::GetInventoryItemData(int32 SlotIndex) const
{
	if (Items().IsValidIndex(SlotIndex) && Items()[SlotIndex].ItemInfo)
	{
		return Items()[SlotIndex].ItemInfo;
	}
	return nullptr;
}

bool UInventoryComponent::SlotEmpty(int SlotIndex)
{
	if (Items().IsValidIndex(SlotIndex) && Items()[SlotIndex].ItemInfo)
	{
		return false;
	}
	return true;
}

void UInventoryComponent::ConstructInventory()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* OwningPawn = Cast<APawn>(Owner);
	if (!OwningPawn) return;

	OwningController = Cast<APlayerController>(OwningPawn->GetController());

	if (!OwningController.IsValid())
	{
		return;
	}

	if (OwningController->IsLocalController() && IsValid(InventoryMenuClass))
	{
		InventoryMenu = CreateWidget<UUserWidget>(OwningController.Get(), InventoryMenuClass);

		if (InventoryMenu)
		{
			InventoryMenu->AddToViewport();
			InventoryMenu->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

int32 UInventoryComponent::GetTotalItemCount(UInventoryItemData* ItemData) const
{
	if (!ItemData) return 0;
	int32 Total = 0;
	for (const FInventorySlot& Slot : Items())
	{
		if (Slot.ItemInfo && Slot.ItemInfo->ItemID == ItemData->ItemID)
		{
			Total += Slot.Quantity;
		}
	}
	return Total;
}
int32 UInventoryComponent::ConsumeItemByPriority(UInventoryItemData* ItemData, int32 AmountToConsume)
{
	if (!GetOwner()->HasAuthority())
	{
		return AmountToConsume; // 客户端直接返回期望值，不动库存
	}
	
	if (!ItemData || AmountToConsume <= 0) return 0;
	//── Step 1: 收集所有含有该物品的格子索引 ──
	TArray<int32> MatchingIndices;
	for (int32 i = 0; i < Items().Num(); i++)
	{
		if (Items()[i].ItemInfo && Items()[i].ItemInfo->ItemID == ItemData->ItemID
			&& Items()[i].Quantity > 0)
		{
			MatchingIndices.Add(i);
		}
	}
	if (MatchingIndices.Num() == 0) return 0;
	// ── Step 2: 按数量升序排序（少的优先扣） ──
	MatchingIndices.Sort([this](const int32 A, const int32 B)
	{
		return Items()[A].Quantity < Items()[B].Quantity;
	});
	// ── Step 3: 依次从最少的格子开始扣 ──
	int32 Remaining = AmountToConsume;
	for (int32 SlotIndex : MatchingIndices)
	{
		if (Remaining <= 0) break;
		FInventorySlot& Slot = Items()[SlotIndex];
		int32 DeductAmount = FMath::Min(Slot.Quantity, Remaining);Slot.Quantity -= DeductAmount;
		Remaining -= DeductAmount;
		// 格子扣空了就清除物品信息
		if (Slot.Quantity <= 0)
		{
			Slot.Quantity = 0;
			Slot.ItemInfo = nullptr;
		}
		SlotArray.MarkSlotDirty(Slot);
	}
	int32 ActualConsumed = AmountToConsume - Remaining;
	UE_LOG(LogTemp, Log, TEXT("ConsumeItemByPriority: Requested=%d, Consumed=%d, Item=%s"),
		AmountToConsume, ActualConsumed, *ItemData->ItemID.ToString());
	return ActualConsumed;
}


// ═════════════════════════════════════════════
//  FInventorySlot 回调实现
// ═════════════════════════════════════════════

void FInventorySlot::PreReplicatedRemove(const FInventorySlotArray& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->OnSlotUpdated();
	}
}

void FInventorySlot::PostReplicatedAdd(const FInventorySlotArray& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		InArraySerializer.OwnerComponent->OnSlotUpdated();
	}
}

void FInventorySlot::PostReplicatedChange(const FInventorySlotArray& InArraySerializer)
{
	if (InArraySerializer.OwnerComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("FInventorySlot::PostReplicatedChange"));
		InArraySerializer.OwnerComponent->OnSlotUpdated();
	}
}
