// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ThidPerson/ApexCharacter.h"

#include "BpexGameplayTags.h"
#include "Players/ShooterPlayerState.h"
#include "Input/BpexInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventorySystem/InventoryComponent.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "Input/BpexCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/CombatComponent.h"
#include "AbilitySystem/LegendAbilityComponent.h"
#include "Players/ShooterPlayerController.h"
#include "Weapon/BulletManagerComponent.h"
#include "Components/CapsuleComponent.h"
#include "Interface/AnimationBlueprintInterface.h"

AApexCharacter::AApexCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	BulletManagerComponent = CreateDefaultSubobject<UBulletManagerComponent>(TEXT("BulletManagerComponent"));

	LegendAbilityComponent = CreateDefaultSubobject<ULegendAbilityComponent>(TEXT("LegendAbilityComponent"));

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void AApexCharacter::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp && DefaultAnimLayerClass)
	{
		MeshComp->LinkAnimClassLayers(DefaultAnimLayerClass);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UnArmedAnimLayerClass not set"));
	}

	UpdateGate(EGate::Jogging);


	if (CombatComp)
	{
		CombatComp->OnWeaponChanged.AddDynamic(this, &AApexCharacter::OnWeaponChanged);
	}
}

void AApexCharacter::OnFiringTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (CurrentGate == EGate::Jogging)
		{
			UpdateGate(EGate::Walking);
		}
	}
	else
	{
		if (CurrentGate == EGate::Walking)
		{
			UpdateGate(EGate::Jogging);
		}
	}
}

void AApexCharacter::RegisterGateTagCallbacks()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	ASC->RegisterGameplayTagEvent(FBpexGameplayTags::Get().State_Action_Firing,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &AApexCharacter::OnFiringTagChanged);
}

void AApexCharacter::OnWeaponChanged(EGun WeaponType)
{
	if (TSubclassOf<UAnimInstance>* Found = AnimLayerMap.Find(WeaponType))
	{
		GetMesh()->LinkAnimClassLayers(*Found);
	}
	UE_LOG(LogTemp, Log, TEXT("AnimLayer → %s"), *UEnum::GetValueAsString(WeaponType));
}

void AApexCharacter::UpdateGate(EGate Gate)
{
	CurrentGate = Gate;
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	if (const FGateSetting* FoundSetting = GateSettings.Find(Gate))
	{
		MovementComp->MaxWalkSpeed = FoundSetting->MaxWalkSpeed;
		MovementComp->MaxAcceleration = FoundSetting->MaxAcceleration;
		MovementComp->BrakingDecelerationWalking = FoundSetting->BrakingDeceleration;
		MovementComp->BrakingFrictionFactor = FoundSetting->BrakingFrictionFactor;
		MovementComp->BrakingFriction = FoundSetting->BrakingFriction;
		MovementComp->bUseSeparateBrakingFriction = FoundSetting->UseSeparateBrakingFriction;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateGate: Do not find GateSetting"));
	}

	if (USkeletalMeshComponent* MyMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MyMesh->GetAnimInstance())
		{
			if (AnimInstance->Implements<UAnimationBlueprintInterface>())
			{
				IAnimationBlueprintInterface::Execute_ReceiveCurrentGate(AnimInstance, CurrentGate);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("AnimInstance did not implement UAnimationBlueprintInterface in Blueprint!"));
			}
		}
	}
}

void AApexCharacter::UpdateGroundDistance()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;

	if (CMC->IsMovingOnGround())
	{
		SendGroundDistance(0.f);
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule) return;
	FVector Start = GetActorLocation();
	float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	FVector End = Start - FVector(0.f, 0.f, 1000.f);
	Start.Z -= HalfHeight;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_Visibility, Params
	);

	SendGroundDistance(bHit ? HitResult.Distance : -1.f);
}

void AApexCharacter::SendGroundDistance(float Distance) const
{
	if (USkeletalMeshComponent* MyMesh = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MyMesh->GetAnimInstance())
		{
			if (AnimInstance->Implements<UAnimationBlueprintInterface>())
			{
				IAnimationBlueprintInterface::Execute_ReceiveGroundDistance(AnimInstance, Distance);
			}
			else
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("AnimInstance did not implement UAnimationBlueprintInterface in Blueprint!"));
			}
		}
	}
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

	if (LegendAbilityComponent)
	{
		LegendAbilityComponent->InitAbilitySystem(AbilitySystemComponent);
	}
	
	//注册 Tag 回调
	RegisterGateTagCallbacks();
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

void AApexCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AApexCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateGroundDistance();
}

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

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AApexCharacter::DoCrouch);
		}

		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AApexCharacter::DoAim);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AApexCharacter::DoUnAim);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AApexCharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AApexCharacter::StopJumping);
		}

		if (SwitchWeaponAction)
		{
			EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this,
			                                   &AApexCharacter::SwitchWeapon);
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
	if (CurrentGate == EGate::Crouching)
	{
		UpdateGate(EGate::Jogging);
		UnCrouch();
	}
	else
	{
		UpdateGate(EGate::Crouching);
		Crouch();
	}
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

void AApexCharacter::DoAim()
{
	UpdateGate(EGate::Walking);
}

void AApexCharacter::DoUnAim()
{
	UpdateGate(EGate::Jogging);
}

void AApexCharacter::SwitchWeapon(const FInputActionValue& Value)
{
	int32 Selection = FMath::TruncToInt(Value.Get<float>());

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	if (Selection == 3)
	{
		CombatComp->Holster();
	}
	else
	{
		CombatComp->EquipSlotWeapon(Selection - 1);
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
