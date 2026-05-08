// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AShooterWeapon;
class UAnimMontage;
class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);


USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AShooterWeapon> WeaponClass;
	
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, EGun, WeaponType);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Combat|Slots")
	int32 NumSlots = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Combat|Sockets")
	FName EquippedSocketName = FName("WeaponEquipped");
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Combat|Runtime")
	TArray<TObjectPtr<AShooterWeapon>> WeaponSlots;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Combat|Runtime")
	int32 ActiveSlotIndex = INDEX_NONE;
	
	
	UPROPERTY(Transient)
	TObjectPtr<AShooterWeapon> CurrentWeapon = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ACharacter> CharacterOwner;

	UPROPERTY(Transient, Replicated)
	TArray<AShooterWeapon*> OwnedWeapons;
	
	//初始武器列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapons")
	TArray<FWeaponInfo> DefaultWeapons;
	
	UPROPERTY(EditAnywhere, Category = "Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.f;

public:
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnWeaponChanged OnWeaponChanged;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void EquipSlotWeapon(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable)
	void Holster();

	UFUNCTION(BlueprintPure)
	AShooterWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	

	UFUNCTION(BlueprintCallable)
	FVector GetWeaponTargetLocation();

protected:
	
	void SpawnAndAttachWeapons();
	
	void AttachWeaponToSocket(AShooterWeapon* Weapon, FName SocketName) const;
	
	USkeletalMeshComponent* GetOwnerMesh() const;
};
