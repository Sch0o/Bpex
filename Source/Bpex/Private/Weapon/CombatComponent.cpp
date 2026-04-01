// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CombatComponent.h"

#include "Characters/ShooterCharacter.h"
#include "Weapon/ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"


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


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterOwner =Cast<ACharacter>(GetOwner());
	
	//枪只能在服务器中生成
	if (DefaultWeaponClass&&CharacterOwner&&CharacterOwner->HasAuthority())
	{
		AddWeaponClass(DefaultWeaponClass);
	}

}

void UCombatComponent::StartFiring() const
{
	if (CurrentWeapon && CharacterOwner)
	{
		CurrentWeapon->StartFiring();
	}
}

void UCombatComponent::StopFiring() const
{
	if (CurrentWeapon && CharacterOwner)
	{
		CurrentWeapon->StopFiring();
	}
}

void UCombatComponent::SwitchWeapon()
{
	if (OwnedWeapons.Num() > 1 && CharacterOwner)
	{
		CurrentWeapon->DeactivateWeapon();
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);
		WeaponIndex = (WeaponIndex == OwnedWeapons.Num() - 1) ? 0 : WeaponIndex + 1;

		CurrentWeapon = OwnedWeapons[WeaponIndex];
		CurrentWeapon->ActivateWeapon();
	}
}

void UCombatComponent::DeactivateCurrentWeapon()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}
}

void UCombatComponent::AddWeaponRecoil(float Recoil)
{
	if (CharacterOwner) CharacterOwner->AddControllerPitchInput(Recoil);
}

void UCombatComponent::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);
	if (!OwnedWeapon && CharacterOwner)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = CharacterOwner;
		SpawnParams.Instigator = CharacterOwner;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(
			WeaponClass, CharacterOwner->GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			AddedWeapon->SetCombatComponent(this);
			if (CurrentWeapon) CurrentWeapon->DeactivateWeapon();
			CurrentWeapon = AddedWeapon;
			OwnedWeapons.Add(AddedWeapon);
			//附着角色
			AttachWeaponMeshes(AddedWeapon);
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void UCombatComponent::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	if (!CharacterOwner) return;

	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);
	Weapon->AttachToActor(CharacterOwner, AttachmentRule);
	
	if (!WeaponSocket.IsValid())
	{
		UE_LOG(LogTemp,Error,TEXT("FirstPersonWeaponSocket is null"));
	}
	Weapon->GetWeaponMesh()->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRule,
	                                                WeaponSocket);
}

void UCombatComponent::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// 1. 第一道防线：检查传入的武器指针是否为空
	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnWeaponActivated: Weapon is nullptr!"));
		return;
	}
    
	// 2. 第二道防线：检查角色和角色的 Mesh 是否有效
	if (IsValid(CharacterOwner) && IsValid(CharacterOwner->GetMesh()))
	{
		// 3. 提取动画类（TSubclassOf 重载了 bool 运算符，可以直接判空）
		if (Weapon->GetAnimInstanceClass())
		{
			CharacterOwner->GetMesh()->SetAnimInstanceClass(Weapon->GetAnimInstanceClass());
		}
		else
		{
			// 方便你在蓝图里忘了配动画时能看到提示，而不是一头雾水
			UE_LOG(LogTemp, Warning, TEXT("OnWeaponActivated: Weapon has no AnimInstanceClass assigned in Blueprint!"));
		}
	}
}

AShooterWeapon* UCombatComponent::GetCurrentWeapon() const
{
	if(!CurrentWeapon)
	{
		UE_LOG(LogTemp,Warning,TEXT("no current weapon"));
	}
	return CurrentWeapon;
}


void UCombatComponent::OnRep_CurrentWeapon(AShooterWeapon* LastWeapon)
{
	if (CurrentWeapon)
	{
		//枪附着在角色上
		AttachWeaponMeshes(CurrentWeapon);
		//设置武器专属动画
		OnWeaponActivated(CurrentWeapon);
	}
}

// Called every frame
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


AShooterWeapon* UCombatComponent::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	if (!WeaponClass) return nullptr;
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (IsValid(Weapon) && Weapon->IsA(WeaponClass)) return Weapon;
	}
	return nullptr;
}
