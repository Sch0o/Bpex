// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexTypes.h"
#include "ThirdPersonCharacter.h"
#include "ApexCharacter.generated.h"

struct FGateSetting;
enum class EGate : uint8;
class ULegendDataAsset;
class ULegendAbilityComponent;
class ULagCompensationComponent;
class UBulletManagerComponent;
class UBpexInputConfig;
struct FGameplayTag;
class UCombatComponent;
class UInventoryComponent;


UCLASS()
class BPEX_API AApexCharacter : public AThirdPersonCharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBulletManagerComponent* BulletManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	ULagCompensationComponent* LagCompensationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	ULegendAbilityComponent* LegendAbilityComponent;

public:
	AApexCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractInventoryAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, Category ="Gate")
	EGate CurrentGate = EGate::Jogging;

	UPROPERTY(EditAnywhere, Category ="Gate")
	TMap<EGate,FGateSetting> GateSettings;

	EGun EquippedGun = EGun::UnArmed;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimLayerClass;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	TMap<EGun,TSubclassOf<UAnimInstance>>AnimLayerMap;
	
	//绑定Firing Tag变化的回调
	void OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	//在ASC初始化后注册回调
	void RegisterGateTagCallbacks();
	
	UFUNCTION()
	void OnWeaponChanged(EGun WeaponType);
	
	void UpdateGate(EGate Gate);
	
	void UpdateGroundDistance();
	
	void SendGroundDistance(float Distance) const;

	virtual void InitAbilityActorInfo() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void PostInitializeComponents() override;

	void DoInteract();

	void DoInteractInventory();

	void DoCrouch();
	
	void DoSprintStart();

	void DoSprintEnd();

	void DoSlideStart();

	void DoSlideEnd();

	void DoAim();

	void DoUnAim();
	
	void SwitchWeapon(const FInputActionValue& Value);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UCombatComponent* GetCombatComponents() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement")
	bool bIsSliding;

	FCollisionQueryParams GetIgnoreCharacterParams() const;

private:
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	void AbilityInputHeld(FGameplayTag InputTag);

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UBpexInputConfig> InputConfig;
};
