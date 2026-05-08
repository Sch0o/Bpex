// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BpexTypes.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class UCombatComponent;
class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

UCLASS(abstract)
class BPEX_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

protected:
	UPROPERTY()
	TObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;
	
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> AnimInstanceClass;
	
	UPROPERTY(EditAnywhere, Category="Weapon|Socket")
	FName MuzzleSocketName;
	
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	TObjectPtr<APawn> PawnOwner;

protected:
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName UnEquippedSocketName;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon")
	EGun WeaponType;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	EAmmoType AmmoType = EAmmoType::None;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ammo")
	int32 ClipAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 MaxClipAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 AmmoPerShot = 1;
	
	AShooterWeapon();

	void SetCombatComponent(UCombatComponent* NewCombatComponent);
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	const TSubclassOf<UAnimInstance>& GetAnimInstanceClass() const;
	
	UCombatComponent* GetCombatComponent() const;

	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
