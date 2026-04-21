// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BpexCharacterMovementComponent.generated.h"

class AApexCharacter;

UENUM(Blueprintable)
enum ECustomMovementMode:uint8
{
	CMOVE_NONE UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName="Sliding"),
	CMOVE_MAX UMETA(Hidden)
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UBpexCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Bpex : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToSprint : 1;

		uint8 Saved_bWantsToSlide : 1;

		virtual bool
		CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetWorkPredictionData_Client_Bpex : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetWorkPredictionData_Client_Bpex(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

public:
	UBpexCharacterMovementComponent();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterSlide();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ExitSlide();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnterSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ExitSprint();
	
	void SlidePressed() { bSafe_WantsToSlide = true; }
	
	void SlideReleased() { bSafe_WantsToSlide = false; }
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding()const;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	
	virtual bool IsMovingOnGround() const override;

protected:
	virtual void BeginPlay() override;

	virtual void InitializeComponent() override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaTime) override;
	
	virtual bool CanCrouchInCurrentState() const override;
	
	bool CanEnterSlide();

	void PhysSlide(float deltaTime, int32 Iterations);

	bool GetSlideSurface(FHitResult& HitResult) const;

	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float MinSlideSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideEnterImpulse = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideFriction = 0.3f;
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideGravityForce = 500.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Slide")
	float SlideGravityModifier = 2.f;

	UPROPERTY(EditDefaultsOnly)
	float Spring_MaxWalkSpeed = 800.f;

	UPROPERTY(EditDefaultsOnly)
	float Walk_MaxWalkSpeed = 600.f;

	UPROPERTY(Transient)
	TObjectPtr<AApexCharacter> ApexCharacterOwner;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	float GetMinSlideSpeed() const;

private:
	bool bSafe_WantsToSlide;
	bool bSafe_WantsToSprint;
};
