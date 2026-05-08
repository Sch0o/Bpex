// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, OwnedWeapons);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	WeaponSlots.Init(nullptr, NumSlots);
	SpawnAndAttachWeapons();
}

void UCombatComponent::EquipSlotWeapon(int32 SlotIndex)
{
	if (SlotIndex == ActiveSlotIndex) return;
	
	if (!WeaponSlots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipSlot: Invalid slot %d"), SlotIndex);
		return;
	}
	
	AShooterWeapon* NewWeapon = WeaponSlots[SlotIndex];
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipSlot: Slot %d is empty"), SlotIndex);
		return;
	}
	
	if (CurrentWeapon)
	{
		AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->UnEquippedSocketName);
	}
	
	AttachWeaponToSocket(NewWeapon,EquippedSocketName);
	CurrentWeapon = NewWeapon;
	ActiveSlotIndex = SlotIndex;
	UE_LOG(LogTemp, Log, TEXT("Equipped Slot[%d]: %s"), SlotIndex, *NewWeapon->GetName());
	
	OnWeaponChanged.Broadcast(CurrentWeapon->WeaponType);
}

void UCombatComponent::Holster()
{
	if (!CurrentWeapon) return;
	AttachWeaponToSocket(CurrentWeapon,CurrentWeapon->UnEquippedSocketName);
	CurrentWeapon = nullptr;
	ActiveSlotIndex = INDEX_NONE;
	UE_LOG(LogTemp, Log, TEXT("Holstered → Unarmed"));
	OnWeaponChanged.Broadcast(EGun::UnArmed);
}

void UCombatComponent::SpawnAndAttachWeapons()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (!OwnerChar) return;
	UWorld* World = GetWorld();
	if (!World) return;

	for (const FWeaponInfo& Info : DefaultWeapons)
	{
		if (!Info.WeaponClass) continue;
		if (!WeaponSlots.IsValidIndex(Info.SlotIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("SlotIndex %d out of range (NumSlots=%d)"), Info.SlotIndex, NumSlots);
			continue;
		}
		// 生成武器 Actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerChar;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AShooterWeapon* NewWeapon = World->SpawnActor<AShooterWeapon>(
			Info.WeaponClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (!NewWeapon) continue;
		WeaponSlots[Info.SlotIndex] = NewWeapon;
		AttachWeaponToSocket(NewWeapon, NewWeapon->UnEquippedSocketName);
		
		UE_LOG(LogTemp, Log, TEXT("Spawned %s → Socket [%s]"), *NewWeapon->GetName(), *(NewWeapon->UnEquippedSocketName.ToString()));
	}
}

void UCombatComponent::AttachWeaponToSocket(AShooterWeapon* Weapon, FName SocketName) const
{
	USkeletalMeshComponent* Mesh = GetOwnerMesh();
	if (!Mesh || !Weapon) return;
	Weapon->AttachToComponent(
		Mesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);
}

USkeletalMeshComponent* UCombatComponent::GetOwnerMesh() const
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	return OwnerChar ? OwnerChar->GetMesh() : nullptr;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UCombatComponent::GetWeaponTargetLocation()
{
	if (!CharacterOwner) return FVector::ZeroVector;

	FHitResult OutHit;
	// Make sure GetFirstPersonCameraComponent is accessible in Character (make it public if needed)
	UCameraComponent* Camera = CharacterOwner->FindComponentByClass<UCameraComponent>();
	if (!Camera) return FVector::ZeroVector;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + (Camera->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}
