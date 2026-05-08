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

	/** Animation montage to play when firing this weapon */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** AnimInstance class to set for the first person character mesh when this weapon is active */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> AnimInstanceClass;

	/** Name of the first person muzzle socket where projectiles will spawn */
	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	/** Distance ahead of the muzzle that bullets will spawn at */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	TObjectPtr<APawn> PawnOwner;


public:
	AShooterWeapon();

	void SetCombatComponent(UCombatComponent* NewCombatComponent);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName UnEquippedSocketName;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon")
	EGun WeaponType;
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	const TSubclassOf<UAnimInstance>& GetAnimInstanceClass() const;
	UCombatComponent* GetCombatComponent() const;

	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;
};
