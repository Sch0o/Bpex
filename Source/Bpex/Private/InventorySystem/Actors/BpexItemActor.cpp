// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Actors/BpexItemActor.h"

#include "InventorySystem/InventoryComponent.h"


// Sets default values
ABpexItemActor::ABpexItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	SetReplicatingMovement(true);
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	RootComponent = ItemMesh;
	
	Quantity = 1;
	ItemData = nullptr;
	
}

void ABpexItemActor::Interact_Implementation(AActor* Interactor)
{
	//无交互对象，无效
	if (!Interactor)return;
	UInventoryComponent* InventoryComponent = Interactor->FindComponentByClass<UInventoryComponent>();
	if (InventoryComponent)
	{
		InventoryComponent->Server_RequestPickUpItem(this);
	}
}

// Called when the game starts or when spawned
void ABpexItemActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABpexItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
