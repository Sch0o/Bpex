// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ThirdPersonCharacter.generated.h"

class UBpexCharacterMovementComponent;
class UGameplayAbility;
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
struct FInputActionValue;
class UInputAction;

UCLASS()
class BPEX_API AThirdPersonCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


public:
	// Sets default values for this character's properties
	AThirdPersonCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* CreateAttributeSet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	void MoveInput(const FInputActionValue& Value);

	void LookInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();


	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GAS|Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	virtual void InitAbilityActorInfo();

	void InitializePrimaryAttributes() const;

	void AddCharacterAbilities();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UBpexCharacterMovementComponent* GetBpexCharacterMovementComponent() const;
	
	UCameraComponent* GetCameraComponent() const;

private:
	UPROPERTY(EditAnywhere, Category="GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};
