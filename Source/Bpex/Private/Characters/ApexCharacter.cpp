// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ThidPerson/ApexCharacter.h"
#include "Players/ShooterPlayerState.h"
#include "Input/BpexInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventorySystem/InventoryComponent.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "Input/BpexCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Players/ShooterPlayerController.h"
#include "Weapon/BulletManagerComponent.h"

// Sets default values
AApexCharacter::AApexCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	
	BulletManagerComponent = CreateDefaultSubobject<UBulletManagerComponent>(TEXT("BulletManagerComponent"));

	//允许角色下蹲
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

// Called when the game starts or when spawned
void AApexCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AApexCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
	check(ShooterPlayerState);
	ShooterPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(ShooterPlayerState, this);
	AbilitySystemComponent = ShooterPlayerState->GetAbilitySystemComponent();
	Cast<UBpexAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AttributeSet = ShooterPlayerState->GetAttributeSet();

	InitializePrimaryAttributes();
}

void AApexCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//在服务器上做
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void AApexCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	//只在客户端执行
	InitAbilityActorInfo();
}

// Called every frame
void AApexCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 画出胶囊体底部位置
	// FVector CapsuleBottom = GetCapsuleComponent()->GetComponentLocation() - FVector(
	// 	0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	// DrawDebugSphere(GetWorld(), CapsuleBottom, 5.f, 8,
	//                 IsLocallyControlled() ? FColor::Green : FColor::Red,
	//                 false, -1.f);
	// 画出Mesh脚底位置  
	FVector MeshFeet = GetMesh()->GetComponentLocation();
	DrawDebugSphere(GetWorld(), MeshFeet, 5.f, 8, IsLocallyControlled() ? FColor::Green : FColor::Red, false, -1.f);
	// 打印两者的Z差值
	// if (GetLocalRole() == ROLE_SimulatedProxy)
	// {
	// 	DrawDebugString(GetWorld(),
	// 	                GetActorLocation() + FVector(0, 0, 100),
	// 	                FString::Printf(TEXT("CapsuleBottom Z: %.1f\nMeshFeet Z: %.1f\nDiff: %.1f\nBaseOffset Z: %.1f"),
	// 	                                CapsuleBottom.Z,
	// 	                                MeshFeet.Z,
	// 	                                MeshFeet.Z - CapsuleBottom.Z,
	// 	                                GetBaseTranslationOffset().Z),
	// 	                nullptr, FColor::White, 0.f, false);
	// }
}

// Called to bind functionality to input
void AApexCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UBpexInputComponent* EnhancedInputComponent = CastChecked<UBpexInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputPressed,
		                                           &ThisClass::AbilityInputReleased, &ThisClass::AbilityInputHeld);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AApexCharacter::DoInteract);

		EnhancedInputComponent->BindAction(InteractInventoryAction, ETriggerEvent::Started, this,
		                                   &AApexCharacter::DoInteractInventory);

		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AApexCharacter::DoSlideStart);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &AApexCharacter::DoSlideEnd);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AApexCharacter::DoSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AApexCharacter::DoSprintEnd);

		//下蹲
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AApexCharacter::DoCrouch);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this,
			                                   &AApexCharacter::DoUnCrouch);
		}
	}
}

UCombatComponent* AApexCharacter::GetCombatComponents() const
{
	return CombatComp;
}

void AApexCharacter::AbilityInputPressed(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputPressed(InputTag);
	}
}

void AApexCharacter::AbilityInputReleased(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputReleased(InputTag);
	}
}

void AApexCharacter::AbilityInputHeld(FGameplayTag InputTag)
{
	if (UBpexAbilitySystemComponent* GAS = Cast<UBpexAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		GAS->AbilityInputHeld(InputTag);
	}
}

FCollisionQueryParams AApexCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams IgnoreParams;
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	IgnoreParams.AddIgnoredActors(CharacterChildren);
	IgnoreParams.AddIgnoredActor(this);
	return IgnoreParams;
}

void AApexCharacter::DoCrouch()
{
	Crouch();
}

void AApexCharacter::DoUnCrouch()
{
	UnCrouch();
}

void AApexCharacter::DoSprintStart()
{
	if (UBpexCharacterMovementComponent* CMC = GetBpexCharacterMovementComponent())
	{
		CMC->EnterSprint();
	}
}

void AApexCharacter::DoSprintEnd()
{
	if (UBpexCharacterMovementComponent* CMC = GetBpexCharacterMovementComponent())
	{
		CMC->ExitSprint();
	}
}

void AApexCharacter::DoSlideStart()
{
	if (UBpexCharacterMovementComponent* CMC = GetBpexCharacterMovementComponent())
	{
		CMC->SlidePressed();
	}
}

void AApexCharacter::DoSlideEnd()
{
	if (UBpexCharacterMovementComponent* CMC = GetBpexCharacterMovementComponent())
	{
		CMC->SlideReleased();
	}
}

void AApexCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AApexCharacter, bIsSliding, COND_None);
}


void AApexCharacter::DoInteract()
{
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController()))
	{
		PC->Interact();
	}
}

void AApexCharacter::DoInteractInventory()
{
	if (InventoryComponent)
	{
		InventoryComponent->InteractInventory();
	}
}
