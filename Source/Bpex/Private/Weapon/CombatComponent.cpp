// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BpexGameplayTags.h"


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
	
	CharacterOwner = Cast<ACharacter>(GetOwner());
	
	WeaponSlots.Init(nullptr, NumSlots);
	SpawnAndAttachWeapons();
	
	SetArmedState(false);
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
	
	SetArmedState(true);
}

void UCombatComponent::Holster()
{
	if (!CurrentWeapon) return;
	AttachWeaponToSocket(CurrentWeapon,CurrentWeapon->UnEquippedSocketName);
	
	CurrentWeapon = nullptr;
	ActiveSlotIndex = INDEX_NONE;
	
	OnWeaponChanged.Broadcast(EGun::UnArmed);
	
	SetArmedState(false);
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

UAbilitySystemComponent* UCombatComponent::GetASC() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

void UCombatComponent::SetArmedState(bool bArmed)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	FBpexGameplayTags & Tags = FBpexGameplayTags::Get();
	if (bArmed)
	{
		// 添加 Armed,移除 Unarmed
		ASC->AddLooseGameplayTag(Tags.State_Armed);
		ASC->RemoveLooseGameplayTag(Tags.State_UnArmed);
	}
	else
	{
		// 添加 Unarmed, 移除 Armed
		ASC->RemoveLooseGameplayTag(Tags.State_Armed);
		ASC->AddLooseGameplayTag(Tags.State_UnArmed);
	}
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
	UCameraComponent* Camera = CharacterOwner->FindComponentByClass<UCameraComponent>();
	if (!Camera) return FVector::ZeroVector;
	
	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + (Camera->GetForwardVector() * MaxAimDistance);
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner);
	
	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}
