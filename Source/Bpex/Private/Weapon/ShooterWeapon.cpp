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
#include "Net/UnrealNetwork.h"

int AShooterWeapon::ConsumeClipAmmo(int32 Amount)
{
	int32 Consumed = FMath::Min(Amount, CurrentClipAmmo);
	CurrentClipAmmo -= Consumed;
	OnClipAmmoChanged.Broadcast(CurrentClipAmmo, MaxClipAmmo);
	return Consumed;
}

int32 AShooterWeapon::AddClipAmmo(int32 Amount)
{
	int32 Space = MaxClipAmmo - CurrentClipAmmo;
	int32 Added = FMath::Min(Amount, Space);
	CurrentClipAmmo += Added;
	OnClipAmmoChanged.Broadcast(CurrentClipAmmo, MaxClipAmmo);
	return Added;
}

void AShooterWeapon::OnRep_CurrentClipAmmo()
{
	OnClipAmmoChanged.Broadcast(CurrentClipAmmo, MaxClipAmmo);
}

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


FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	const FVector MuzzleLoc = WeaponMesh->GetSocketLocation(MuzzleSocketName);

	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation);
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

void AShooterWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterWeapon, CurrentClipAmmo);
}


const TSubclassOf<UAnimInstance>& AShooterWeapon::GetAnimInstanceClass() const
{
	return AnimInstanceClass;
}

UCombatComponent* AShooterWeapon::GetCombatComponent() const
{
	return CombatComp;
}
