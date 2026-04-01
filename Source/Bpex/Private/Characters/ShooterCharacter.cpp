// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/ShooterCharacter.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "Weapon/CombatComponent.h"
#include "Input/BpexInputComponent.h"
#include "InventorySystem/InventoryComponent.h"
#include "Players/ShooterPlayerController.h"
#include "Players/ShooterPlayerState.h"


float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	return Damage;
}

AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	
	//允许角色下蹲
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UBpexInputComponent* EnhancedInputComponent = CastChecked<UBpexInputComponent>(PlayerInputComponent))
	{
		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this,
		                                   &AShooterCharacter::DoSwitchWeapon);

		EnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputPressed,
		                                           &ThisClass::AbilityInputReleased, &ThisClass::AbilityInputHeld);
		
		EnhancedInputComponent->BindAction(InteractAction,ETriggerEvent::Started,this,&AShooterCharacter::DoInteract);
		
		EnhancedInputComponent->BindAction(InteractInventoryAction,ETriggerEvent::Started,this,&AShooterCharacter::DoInteractInventory);
		
		//下蹲
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction,ETriggerEvent::Started, this,&AShooterCharacter::DoCrouch);
			EnhancedInputComponent->BindAction(CrouchAction,ETriggerEvent::Completed,this,&AShooterCharacter::DoUnCrouch);
		}
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AShooterCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
	check(ShooterPlayerState);
	ShooterPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(ShooterPlayerState, this);
	AbilitySystemComponent = ShooterPlayerState->GetAbilitySystemComponent();
	Cast<UBpexAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AttributeSet = ShooterPlayerState->GetAttributeSet();

	InitializePrimaryAttributes();

	if (AttributeSet)
	{
		UBpexAttributeSet* BpexAttributes = Cast<UBpexAttributeSet>(AttributeSet);
		BpexAttributes->OnOutOfHealth.AddDynamic(this, &AShooterCharacter::Die);
	}
}


void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	Super::DoAim(Yaw, Pitch);
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	Super::DoMove(Right, Forward);
}

void AShooterCharacter::DoJumpStart()
{
	
	Super::DoJumpStart();

}

void AShooterCharacter::DoJumpEnd()
{
	Super::DoJumpEnd();
}

void AShooterCharacter::DoCrouch()
{
	Crouch();
}

void AShooterCharacter::DoUnCrouch()
{
	UnCrouch();
}

void AShooterCharacter::DoInteract()
{
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController()))
	{
		PC->Interact();
	}
}

void AShooterCharacter::DoInteractInventory()
{
	if (InventoryComponent)
	{
		InventoryComponent->InteractInventory();
	}
}

void AShooterCharacter::DoStartFiring()
{
	CombatComp->StartFiring();
}

void AShooterCharacter::DoStopFiring()
{
	if (CombatComp)
	{
		CombatComp->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (CombatComp)
	{
		CombatComp->SwitchWeapon();
	}
}

void AShooterCharacter::Die(AActor* Killer)
{
	if (CombatComp)
	{
		CombatComp->DeactivateCurrentWeapon();
	}

	// grant the death tag to the character
	Tags.Add(DeathTag);

	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable controls
	DisableInput(nullptr);
	

	// call the BP handler
	BP_OnDeath();

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}


UCombatComponent* AShooterCharacter::GetCombatComponents() const
{
	return CombatComp;
}

void AShooterCharacter::AbilityInputPressed(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputPressed(InputTag);
	}
}

void AShooterCharacter::AbilityInputReleased(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputReleased(InputTag);
	}
}

void AShooterCharacter::AbilityInputHeld(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputHeld(InputTag);
	}
}
