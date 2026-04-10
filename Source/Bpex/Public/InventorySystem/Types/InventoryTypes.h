#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "InventoryTypes.generated.h"


struct FInventorySlotArray;
class UInventoryComponent;

UENUM(BlueprintType)
enum class EItemType:uint8
{
	Weapon UMETA(DisplayName = "武器"),
	Ammo UMETA(DisplayName = "子弹"),
	Consumable UMETA(DisplayName = "消耗品"),
	Attachment UMETA(DisplayName = "配件"),
	Equipment UMETA(DisplayName = "装备")
};

UCLASS(Blueprintable, BlueprintType)
class BPEX_API UInventoryItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Data")
	EItemType ItemType = EItemType::Ammo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Data")
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Data")
	FText ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Data")
	UTexture2D* ItemIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Data", meta = (ClampMin=1))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Consumable")
	TSubclassOf<UGameplayAbility> UseAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Consumable")
	TSubclassOf<UGameplayEffect> ItemEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Consumable")
	FGameplayTag UseEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS|Consumable")
	float UseDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item")
	TSubclassOf<AActor> ItemClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Sorting")
	int SortPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	TSubclassOf<UGameplayEffect> ModifyAttributeEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	FGameplayTag MagnitudeTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GAS")
	int32 DefaultDropAmount =1;
};

USTRUCT(Blueprintable)
struct FInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()
	FInventorySlot() : ItemInfo(nullptr), Quantity(0){}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UInventoryItemData> ItemInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin=0))
	int32 Quantity;

	//当这个元素在客户端被同步移除之前
	void PreReplicatedRemove(const FInventorySlotArray& InArraySerializer);
	//当这个元素在客户端被同步添加时
	void PostReplicatedAdd(const FInventorySlotArray& InArraySerializer);
	//当这个元素在客户端被同步改变时
	void PostReplicatedChange(const FInventorySlotArray& InArraySerializer);
};

USTRUCT(Blueprintable)
struct FInventorySlotArray : public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FInventorySlot> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> OwnerComponent = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FInventorySlot, FInventorySlotArray>(
			Items, DeltaParms, *this);
	}

	void MarkSlotDirty(FInventorySlot& Slot)
	{
		MarkItemDirty(Slot);
	}

	void MarkAllDirty()
	{
		MarkArrayDirty();
	}
};

//让网络层将其识别为FastArray
template <>
struct TStructOpsTypeTraits<FInventorySlotArray>
	: public TStructOpsTypeTraitsBase2<FInventorySlotArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


USTRUCT(BlueprintType)
struct FUI_ItemUseMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Message")
	float UseDuration = 0.0f;
};

UCLASS(BlueprintType)
class BPEX_API UItemUseContext : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UInventoryItemData> ItemData;

	UPROPERTY()
	int32 SlotIndex;
};
