// Copyright Epic Games, Inc. All Rights Reserved.


#include "Weapon/ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Weapon/CombatComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionProfileName(FName("NoCollision"));
	
	bReplicates = true;
}

void AShooterWeapon::SetCombatComponent(UCombatComponent* NewCombatComponent)
{
	CombatComp = NewCombatComponent;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();
}


void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	Destroy();
}


FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	const FVector MuzzleLoc = WeaponMesh->GetSocketLocation(MuzzleSocketName);


	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// find the aim rotation vector while applying some variance to the target 
	//const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation);
	// return the built transform
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}


const TSubclassOf<UAnimInstance>& AShooterWeapon::GetAnimInstanceClass() const
{
	return AnimInstanceClass;
}

UCombatComponent* AShooterWeapon::GetCombatComponent() const
{
	return CombatComp;
}
