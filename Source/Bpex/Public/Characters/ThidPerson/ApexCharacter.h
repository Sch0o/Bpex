// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ThirdPersonCharacter.h"
#include "ApexCharacter.generated.h"

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

public:
	// Sets default values for this character's properties
	AApexCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
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
