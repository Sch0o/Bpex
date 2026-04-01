// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class AApexCharacter;
class UInvHUDWidget;
class UShooterUI;
class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;

UCLASS(abstract, config="Game")
class BPEX_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;
	
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AApexCharacter> CharacterClass;

protected:
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);
	
	void TraceForItem();
	
	virtual void OnRep_PlayerState();
	
	UPROPERTY(EditDefaultsOnly, Category="Inventroy|UI")
	double TraceLength = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Inventroy")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel;
	
	TWeakObjectPtr<AActor>ThisActor;
	TWeakObjectPtr<AActor>LastActor;
	
public:
	void Interact() const;
	
	void TryInitMVVM();
};
