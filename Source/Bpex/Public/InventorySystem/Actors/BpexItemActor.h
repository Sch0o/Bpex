// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventorySystem/Interact/InteractableInterface.h"
#include "BpexItemActor.generated.h"

class UInventoryItemData;
class USphereComponent;

UCLASS()
class BPEX_API ABpexItemActor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABpexItemActor();
	
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	//可视化组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	UStaticMeshComponent* ItemMesh;

	//碰撞组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	USphereComponent* InteractionsSphere;

public:
	//物品类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Data", meta = (ExposeOnSpawn = "true"))
	UInventoryItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Data", meta = (ExposeOnSpawn = "true"))
	int32 Quantity;

	UInventoryItemData* GetItemData() const { return ItemData; }
	int32 GetQuantity() const { return Quantity; }
	void SetQuantity(int32 NewQuantity) { Quantity = NewQuantity; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
