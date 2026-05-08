// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BpexGameplayTags.h"
#include "InventorySystem/InventoryComponent.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, OwnedWeapons);
	DOREPLIFETIME(UCombatComponent, WeaponSlots);
	DOREPLIFETIME(UCombatComponent, ActiveSlotIndex);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ACharacter>(GetOwner());

	//获取InventoryComponent引用
	if (CharacterOwner)
	{
		InventoryComp = CharacterOwner->FindComponentByClass<UInventoryComponent>();
	}

	//初始化武器槽。生成默认武器
	WeaponSlots.Init(nullptr, NumSlots);
	SpawnAndAttachWeapons();

	//设置状态为未装备武器
	SetArmedState(false);
}

int32 UCombatComponent::ConsumeClipAmmo(int32 Amount)
{
	if (!CurrentWeapon) return 0;
	int32 Consumed = CurrentWeapon->ConsumeClipAmmo(Amount);
	if (GetOwner()->HasAuthority()&&Consumed > 0)
	{
		BroadcastAmmoUI();
	}
	return Consumed;
}

void UCombatComponent::Reload()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_Reload();
		return;
	} 
	if (!CurrentWeapon&&!InventoryComp) return;
	
	//弹匣满则不换弹
	if (CurrentWeapon->IsClipFull()) return;
	
	EAmmoType Type = CurrentWeapon->AmmoType;
	int32 AmmoNeeded = CurrentWeapon->MaxClipAmmo - CurrentWeapon->CurrentClipAmmo;
	int32 AmmoAvailable = InventoryComp->GetAmmoCount(Type);
	int32 AmmoToLoad = FMath::Min(AmmoNeeded, AmmoAvailable);
	
	//背包无子弹，无法换弹
	if (AmmoToLoad <= 0) return;
	//从背包中取子弹
	int32 Consumed = InventoryComp->ConsumeAmmo(Type, AmmoToLoad);
	//填入弹匣
	CurrentWeapon->AddClipAmmo(Consumed);
	
	BroadcastAmmoUI();
}

void UCombatComponent::BroadcastAmmoUI()
{
	if (!CurrentWeapon) return;
	int32 Reserve = 0;
	if (InventoryComp)
	{
		Reserve = InventoryComp->GetAmmoCount(CurrentWeapon->AmmoType);
	}
	OnAmmoUIUpdated.Broadcast(CurrentWeapon->CurrentClipAmmo, Reserve);
}

EAmmoType UCombatComponent::GetCurrentAmmoType() const
{
	return CurrentWeapon ? CurrentWeapon->AmmoType : EAmmoType::None;
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
		//解绑新武器
		UnbindWeaponAmmoDelegate(CurrentWeapon);
		//将当前武器放到unequipped时位置
		AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->UnEquippedSocketName);
	}
	//绑定新武器
	BindWeaponAmmoDelegate(CurrentWeapon);
	//手持新武器
	AttachWeaponToSocket(NewWeapon, EquippedSocketName);
	CurrentWeapon = NewWeapon;
	ActiveSlotIndex = SlotIndex;
	UE_LOG(LogTemp, Log, TEXT("Equipped Slot[%d]: %s"), SlotIndex, *NewWeapon->GetName());

	//通知改变动画层
	OnWeaponChanged.Broadcast(CurrentWeapon->WeaponType);
	//通知ui更新弹药数
	BroadcastAmmoUI();
	//更新角色状态为持有武器
	SetArmedState(true);
}

void UCombatComponent::Holster()
{
	if (!CurrentWeapon) return;
	
	UnbindWeaponAmmoDelegate(CurrentWeapon);
	AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->UnEquippedSocketName);

	CurrentWeapon = nullptr;
	ActiveSlotIndex = INDEX_NONE;

	OnWeaponChanged.Broadcast(EGun::UnArmed);
	OnAmmoUIUpdated.Broadcast(1, 0);

	SetArmedState(false);
}

void UCombatComponent::Server_Reload_Implementation()
{
	Reload();
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

		UE_LOG(LogTemp, Log, TEXT("Spawned %s → Socket [%s]"), *NewWeapon->GetName(),
		       *(NewWeapon->UnEquippedSocketName.ToString()));
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

	FBpexGameplayTags& Tags = FBpexGameplayTags::Get();
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

void UCombatComponent::BindWeaponAmmoDelegate(AShooterWeapon* Weapon)
{
	if (!Weapon) return;
	Weapon->OnClipAmmoChanged.AddDynamic(this, &UCombatComponent::OnWeaponClipAmmoChanged);
}

void UCombatComponent::UnbindWeaponAmmoDelegate(AShooterWeapon* Weapon)
{
	if (!Weapon)return;
	Weapon->OnClipAmmoChanged.RemoveDynamic(this, &UCombatComponent::OnWeaponClipAmmoChanged);
}

void UCombatComponent::OnWeaponClipAmmoChanged(int32 NewClip, int32 MaxClip)
{
	BroadcastAmmoUI();
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
