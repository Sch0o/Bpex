// Copyright Epic Games, Inc. All Rights Reserved.


#include "Players//ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/ShooterCharacter.h"
#include "GameFramework/PlayerState.h"
#include "InventorySystem/InvItemComponent.h"
#include "InventorySystem/Interact/InteractableInterface.h"
#include "UI/BpexHUD.h"

void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TraceForItem();
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AShooterPlayerController::TraceForItem()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport))return;
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D ViewportCenter = ViewportSize / 2;
	FVector TraceStart;
	FVector Forward;
	if (!UGameplayStatics::DeprojectScreenToWorld(this, ViewportCenter, TraceStart, Forward)) return;
	const FVector TraceEnd = TraceStart + Forward * TraceLength;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel);

	LastActor = ThisActor;
	ThisActor = HitResult.GetActor();

	ABpexHUD* HUD = GetHUD<ABpexHUD>();

	if (!ThisActor.IsValid())
	{
		if (HUD)
		{
			HUD->HidePickupMessage();
		}
	}

	if (ThisActor == LastActor) return;

	if (ThisActor.IsValid())
	{
		UInvItemComponent* ItemComponent = ThisActor->FindComponentByClass<UInvItemComponent>();
		if (!IsValid(ItemComponent)) return;

		if (HUD)
		{
			HUD->ShowPickupMessage(ItemComponent->GetItemInfo());
		}
	}
}

void AShooterPlayerController::TryInitMVVM()
{
	APlayerState* PS = GetPlayerState<APlayerState>();
	if (!PS) return;
	
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS);
	if (ASI)
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (ASC)
		{
			if (ABpexHUD* HUD = GetHUD<ABpexHUD>())
			{
				HUD->BP_OnAbilitySystemInitialized(ASC);
			}
		}
	}
}

void AShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	TryInitMVVM();
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	TryInitMVVM();
}

void AShooterPlayerController::Interact() const
{
	if (ThisActor.IsValid())
	{
		if (APawn* ControllerPawn = GetPawn())
		{
			UE_LOG(LogTemp, Log, TEXT("Interacting in Player Controller"));
			IInteractableInterface::Execute_Interact(ThisActor.Get(), ControllerPawn);
		}
	}
}


void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);
	
}
