// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Types/InventoryTypes.h"
#include "InventoryComponent.generated.h"


class ABpexItemActor;
class UInventoryItemData;
struct FInventorySlot;
class UInventoryBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUseStartedSignature, float, UseDuration);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	TWeakObjectPtr<APlayerController> OwningController;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	FInventorySlotArray SlotArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	int32 MaxCapacity;

	/** * 尝试添加物品到背包
	 * @param ItemToAdd 想要添加的物品数据
	 * @param QuantityToAdd 想要添加的数量 (传入引用，方便修改剩余数量)
	 * @return 成功放入背包的数量。如果背包满了，通过判断返回值和原数量即可知道还剩多少没捡起来
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	int32 TryAddItem(UInventoryItemData* ItemToAdd, int32& QuantityToAdd);

	/** * 尝试使用背包中的物品
	* @param SlotIndex 想要使用的物品的槽位Index
	*/
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void TryUseItem(int32 SlotIndex);

	/** * 尝试丢弃背包中的物品
	* @param SlotIndex 想要丢弃的物品的槽位Index
	*/
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void TryDropItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category="Inventory|Operations")
	void ConsumeItemInSlot(int32 SlotIndex, int32 ConsumeQuantity = 1);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ExpandInventory(int32 AdditionalSlots);

	UFUNCTION(Server, Reliable, WithValidation, Blueprintable, Category="Inventory")
	void Server_RequestPickUpItem(ABpexItemActor* ItemToPickUp);

	UFUNCTION(Server, Reliable, WithValidation, Blueprintable, Category="Inventory")
	void Server_RequestDropItem(int32 SlotIndex);

	//向服务端请求使用item
	UFUNCTION(Server, Reliable, WithValidation, Category="Inventory")
	void Server_RequestUseItem(int32 SlotIndex);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category="Inventory|Events")
	FOnInventoryUpdated OnInventoryUpdated;

	void InteractInventory();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	float GetInventoryItemUseDuration(int32 SlotIndex) const;

	UInventoryItemData* GetInventoryItemData(int32 SlotIndex) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemUseStartedSignature OnItemUseStarted;

	UFUNCTION()
	void OnSlotUpdated();

	TArray<FInventorySlot>& Items() { return SlotArray.Items; }

	const TArray<FInventorySlot>& Items() const { return SlotArray.Items; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	float ThrowForce = 600.0f;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SlotEmpty(int SlotIndex);

private:
	void ConstructInventory();

	void GetLastSameItemSlotIndex(int32& SlotIndex, UInventoryItemData* ItemData);

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<UUserWidget> InventoryMenuClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> InventoryMenu;

	bool bIsShowingInventoryMenu = false;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FGameplayTag InventoryOpenTag;
};
