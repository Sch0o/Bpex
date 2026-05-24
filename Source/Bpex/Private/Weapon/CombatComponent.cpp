// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BpexGameplayTags.h"
#include "AbilitySystem/Ability/BpexGameplayAbility.h"
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
	if (GetOwner()->HasAuthority())
	{
		WeaponSlots.Init(nullptr, NumSlots);
		SpawnAndAttachWeapons();
	}

	//设置状态为未装备武器
	SetArmedState(false);
}

bool UCombatComponent::IsSlotValid(int SlotIndex) const
{
	return WeaponSlots.IsValidIndex(SlotIndex) && WeaponSlots[SlotIndex] != nullptr;
}

int32 UCombatComponent::ConsumeClipAmmo(int32 Amount)
{
	if (!CurrentWeapon) return 0;
	int32 Consumed = CurrentWeapon->ConsumeClipAmmo(Amount);
	return Consumed;
}

void UCombatComponent::Server_ConsumeClipAmmo_Implementation(int32 Amount)
{
	ConsumeClipAmmo(Amount);
}

void UCombatComponent::Reload()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_Reload();
		return;
	}
	if (!CurrentWeapon && !InventoryComp) return;

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
	//必须是新槽位
	if (SlotIndex == ActiveSlotIndex) return;
	//验证槽位是否合法
	if (!WeaponSlots.IsValidIndex(SlotIndex)) return;
	
	AShooterWeapon* NewWeapon = WeaponSlots[SlotIndex];
	if (!NewWeapon) return;
	// 收起当前武器
	if (CurrentWeapon)
	{
		UnbindWeaponAmmoDelegate(CurrentWeapon);
		AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->UnEquippedSocketName);
		ClearCurrentWeaponAbilities();
	}
	// 装备新武器
	BindWeaponAmmoDelegate(NewWeapon);
	AttachWeaponToSocket(NewWeapon, EquippedSocketName);
	GrantWeaponAbilities(SlotIndex);
	CurrentWeapon = NewWeapon;
	ActiveSlotIndex = SlotIndex;
	
	// 服务器本地表现
	OnWeaponChanged.Broadcast(CurrentWeapon->WeaponType);
	BroadcastAmmoUI();
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
	OnAmmoUIUpdated.Broadcast(0, 0);
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

void UCombatComponent::ClearCurrentWeaponAbilities()
{
	UAbilitySystemComponent* ASC = GetASC();
	
	if (!ASC || !GetOwner()->HasAuthority()) 
	{
		return; 
	}
	
	for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponAbilityHandles)
	{
		if (Handle.IsValid())
		{
			ASC->CancelAbilityHandle(Handle);
			ASC->ClearAbility(Handle);
		}
	}
	CurrentWeaponAbilityHandles.Empty();
}

void UCombatComponent::GrantWeaponAbilities(int32 TargetSlot)
{
	UAbilitySystemComponent* ASC = GetASC();
	AShooterWeapon* NewWeapon = WeaponSlots[TargetSlot];
    
	if (!ASC || !NewWeapon) return;
	
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<UGameplayAbility> AbilityClass : NewWeapon->WeaponAbilities)
		{
			if (AbilityClass)
			{
				FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, NewWeapon);
				
				if (const UBpexGameplayAbility* AbilityCDO = Cast<UBpexGameplayAbility>(AbilityClass.GetDefaultObject()))
				{
					if (AbilityCDO->StartupInputTag.IsValid())
					{
						Spec.GetDynamicSpecSourceTags().AddTag(AbilityCDO->StartupInputTag);
					}
				}
				
				FGameplayAbilitySpecHandle GrantedHandle = ASC->GiveAbility(Spec);
				CurrentWeaponAbilityHandles.Add(GrantedHandle);
			}
		}
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

void UCombatComponent::OnRep_ActiveSlotIndex()
{
	
	//ActiveSlotIndex更新时，CurrentWeapon和ActiveSlotIndex相对应则说明预测成功
	if (CurrentWeapon && WeaponSlots.IsValidIndex(ActiveSlotIndex)&&CurrentWeapon == WeaponSlots[ActiveSlotIndex])
	{
		return;
	}
	
	//状态不一致，需要纠正,以服务器为准
	if (CurrentWeapon)
	{
		UnbindWeaponAmmoDelegate(CurrentWeapon);
		AttachWeaponToSocket(CurrentWeapon,CurrentWeapon->UnEquippedSocketName);	
	}

	CurrentWeapon = nullptr;

	if (WeaponSlots.IsValidIndex(ActiveSlotIndex) && WeaponSlots[ActiveSlotIndex])
	{
		CurrentWeapon = WeaponSlots[ActiveSlotIndex];
		BindWeaponAmmoDelegate(CurrentWeapon);
		OnWeaponChanged.Broadcast(CurrentWeapon->WeaponType);
		BroadcastAmmoUI();
		SetArmedState(true);
	}
	else
	{
		OnWeaponChanged.Broadcast(EGun::UnArmed);
		OnAmmoUIUpdated.Broadcast(0, 0);
		SetArmedState(false);
	}
}
