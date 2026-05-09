// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


enum class EAmmoType : uint8;
class UAbilitySystemComponent;
class AShooterWeapon;
class UAnimMontage;
class AShooterCharacter;
class UInventoryComponent;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoUIUpdated, int32, ClipAmmo, int32, ReserveAmmo);

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
	
	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlotIndex)
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
	//弹药UI更新委托
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnAmmoUIUpdated OnAmmoUIUpdated;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnWeaponChanged OnWeaponChanged;
	
	//扣弹夹子弹
	int32 ConsumeClipAmmo(int32 Amount =1);
	
	UFUNCTION(Server, Reliable)
	void Server_ConsumeClipAmmo(int32 Amount);
	
	//换弹
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Reload();
	
	//刷新子弹UI
	void BroadcastAmmoUI();
	
	//获取当前武器子弹类型
	EAmmoType GetCurrentAmmoType() const;
	
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
	
	UFUNCTION(Server,Reliable)
	void Server_EquipSlotWeapon(int32 SlotIndex);
	
	void Internal_EquipSlotWeapon(int32 SlotIndex);
	
	UFUNCTION(Server,Reliable)
	void Server_Holster();
	
	void Internal_Holster();
	
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp = nullptr;
	
	UFUNCTION()
	void OnRep_ActiveSlotIndex();
	
	UFUNCTION(Server,Reliable)
	void Server_Reload();
	
	void SpawnAndAttachWeapons();
	
	void AttachWeaponToSocket(AShooterWeapon* Weapon, FName SocketName) const;
	
	USkeletalMeshComponent* GetOwnerMesh() const;
	
	UAbilitySystemComponent* GetASC() const;
	
	void SetArmedState(bool bArmed);
	
private:
	void BindWeaponAmmoDelegate(AShooterWeapon* Weapon);
	void UnbindWeaponAmmoDelegate(AShooterWeapon* Weapon);
	
	UFUNCTION()
	void OnWeaponClipAmmoChanged(int32 NewClip, int32 MaxClip);
	
};
