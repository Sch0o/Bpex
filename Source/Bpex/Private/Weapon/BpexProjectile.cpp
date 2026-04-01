// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BpexProjectile.h"
#include "Components/SphereComponent.h"


// Sets default values
ABpexProjectile::ABpexProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABpexProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
		Mesh->IgnoreActorWhenMoving(GetInstigator(), true);
	}

}

// Called every frame
void ABpexProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

