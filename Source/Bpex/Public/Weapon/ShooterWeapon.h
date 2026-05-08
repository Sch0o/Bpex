// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BpexTypes.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "AmmoTypes.h"
#include "ShooterWeapon.generated.h"

class UCombatComponent;
class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClipAmmoChanged, int32, NewClip, int32, MaxClip);

UCLASS(abstract)
class BPEX_API AShooterWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

protected:
	UPROPERTY()
	TObjectPtr<UCombatComponent> CombatComp;
	
	UPROPERTY(EditAnywhere, Category="Weapon|Socket")
	FName MuzzleSocketName;
	
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 0.0f;

	TObjectPtr<APawn> PawnOwner;
	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_CurrentClipAmmo();

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName UnEquippedSocketName;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Weapon")
	EGun WeaponType;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	EAmmoType AmmoType = EAmmoType::None;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentClipAmmo, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 CurrentClipAmmo = 30;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo")
	int32 MaxClipAmmo = 30;
	
	int32 ConsumeClipAmmo(int32 Amount = 1);
	int32 AddClipAmmo(int32 Amount);
	
	bool IsClipEmpty()const {return CurrentClipAmmo <=0; }
	
	bool IsClipFull()const {return CurrentClipAmmo >= MaxClipAmmo;}
	
	UPROPERTY(BlueprintAssignable)
	FOnClipAmmoChanged OnClipAmmoChanged;
	
	AShooterWeapon();

	void SetCombatComponent(UCombatComponent* NewCombatComponent);
	
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	
	UCombatComponent* GetCombatComponent() const;

	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
