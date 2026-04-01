// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BpexCharacter.h"
#include  "GameplayTags.h"
#include "ShooterCharacter.generated.h"

class UInventoryComponent;
struct FGameplayTag;
class UCombatComponent;
class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class UBpexInputConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class BPEX_API AShooterCharacter : public ABpexCharacter
{
	GENERATED_BODY()

	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* InventoryComponent;

protected:
	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractInventoryAction;
	
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

public:
	/** Damaged delegate */
	FDamagedDelegate OnDamaged;
	
	AShooterCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void InitAbilityActorInfo() override;

public:
	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

public:
	/** Handles aim inputs from either controls or UI interfaces */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Handles move inputs from either controls or UI interfaces */
	virtual void DoMove(float Right, float Forward) override;

	/** Handles jump start inputs from either controls or UI interfaces */
	virtual void DoJumpStart() override;

	/** Handles jump end inputs from either controls or UI interfaces */
	virtual void DoJumpEnd() override;
	
	void DoCrouch();
	
	void DoUnCrouch();

	void DoInteract();

	void DoInteractInventory();

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

protected:
	UFUNCTION()
	void Die(AActor* Killer);

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();

public:
	/** Returns true if the character is dead */
	//bool IsDead() const;

	UCombatComponent* GetCombatComponents() const;

private:
	void AbilityInputPressed(FGameplayTag InputTag);

	void AbilityInputReleased(FGameplayTag InputTag);

	void AbilityInputHeld(FGameplayTag InputTag);

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UBpexInputConfig> InputConfig;
};
