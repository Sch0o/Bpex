// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThirdPersonCharacter.h"
#include "ApexCharacter.generated.h"

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
	UBulletManagerComponent*BulletManagerComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	ULagCompensationComponent* LagCompensationComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	ULegendAbilityComponent * LegendAbilityComponent;

public:
	AApexCharacter(const FObjectInitializer& ObjectInitializer);
	
	//角色独有的技能
	UPROPERTY(EditDefaultsOnly, Category = "Legend")
	TObjectPtr<ULegendDataAsset> LegendData;

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
	
	virtual void InitAbilityActorInfo() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;
	
	virtual void PostInitializeComponents() override;
	
	void DoInteract();
	
	void DoInteractInventory();
	
	void DoCrouch();
	
	void DoUnCrouch();
	
	void DoSprintStart();
	
	void DoSprintEnd() ;
	
	void DoSlideStart() ;
	
	void DoSlideEnd() ;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UCombatComponent* GetCombatComponents() const;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement")
	bool bIsSliding;
	
	FCollisionQueryParams GetIgnoreCharacterParams() const;
	
private:
	// ================= GAS 输入绑定 =================
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	void AbilityInputHeld(FGameplayTag InputTag);

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UBpexInputConfig> InputConfig;
	

};
