// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AShooterWeapon;
class UAnimMontage;
class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();
	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName WeaponSocket;
	
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	UPROPERTY(Transient)
	ACharacter* CharacterOwner;

	UPROPERTY(Transient,Replicated)
	TArray<AShooterWeapon*> OwnedWeapons;

	UPROPERTY(Transient,ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AShooterWeapon> CurrentWeapon;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AShooterWeapon>DefaultWeaponClass;
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AShooterWeapon*LastWeapon);


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void StartFiring() const;
	void StopFiring() const;
	void SwitchWeapon();
	void DeactivateCurrentWeapon();
	
	void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass);
	void AttachWeaponMeshes(AShooterWeapon* Weapon);
	void AddWeaponRecoil(float Recoil);
	FVector GetWeaponTargetLocation();
	void OnWeaponActivated(AShooterWeapon* Weapon);
	
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	AShooterWeapon*GetCurrentWeapon()const;
protected:
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;
};
