// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BpexGameplayTags.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "Weapon/CombatComponent.h"
#include "InventorySystem/InventoryComponent.h"


void UShooterViewModel::SetHealthPercent(float NewPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, NewPercent);
}

void UShooterViewModel::SetReserveAmmo(int32 NewReserverAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(ReserveAmmo, NewReserverAmmo);
}

void UShooterViewModel::SetClipAmmo(int32 NewClipAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(ClipAmmo, NewClipAmmo);
}

void UShooterViewModel::SetIsUsingItem(bool bNewState)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsUsingItem, bNewState);
}

void UShooterViewModel::SetCurrentUseDuration(float NewDuration)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentUseDuration, NewDuration);
}

void UShooterViewModel::InitializeViewModel(APlayerController* PC)
{
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UShooterViewModel::InitializeViewModel:: PlayerController is null"));
		return;
	}
	PlayerController = PC;
	PC->OnPossessedPawnChanged.AddDynamic(this, &UShooterViewModel::HandlePawnChanged);
	
	if (APawn* CurrentPawn = PC->GetPawn())
	{
		HandlePawnChanged(nullptr, CurrentPawn);
	}
	
}

void UShooterViewModel::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Log, TEXT("UShooterViewModel::HandlePawnChanged:: NewPawn is null"));
		return;
	}
	if (UInventoryComponent* InventoryComp = NewPawn->FindComponentByClass<UInventoryComponent>())
	{
		
		InitializeInventory(InventoryComp);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("UShooterViewModel::HandlePawnChanged:: InventoryComp is null"));
	}
	
	if (UCombatComponent* Combat = NewPawn->FindComponentByClass<UCombatComponent>())
	{
		InitializeCombat(Combat);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("UShooterViewModel::HandlePawnChanged:: CombatComponent is null"));
	}
	
}

void UShooterViewModel::InitializeASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("ViewModel Init fail:ASC is null"))
		return;
	}
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetHealthAttribute()).AddUObject(
		this, &UShooterViewModel::HealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetMaxHealthAttribute()).AddUObject(
		this, &UShooterViewModel::MaxHealthChanged);
	
	UpdateHealthPercent();

	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("State.UI.UsingItem"));
	ASC->RegisterGameplayTagEvent(RootTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &UShooterViewModel::OnAnyGameplayTagChanged);
}

void UShooterViewModel::InitializeInventory(UInventoryComponent* InIC)
{
	InventoryComponent = InIC;
	if (!InIC)
	{
		UE_LOG(LogTemp, Error, TEXT("ViewModel Init fail:InventoryComponent is null"))
		return;
	}
	InventoryComponent->OnItemUseStarted.AddDynamic(this, &UShooterViewModel::HandleItemUseStarted);
	
	InventoryComponent->OnAmmoChanged.AddDynamic(this, &UShooterViewModel::HandleAmmoChanged);
	
}

void UShooterViewModel::InitializeCombat(UCombatComponent* InCombat)
{
	CombatComponent = InCombat;
	if (!CombatComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ViewModel Init fail:InventoryComponent is null"))
		return;
	}
	CombatComponent->OnAmmoUIUpdated.AddDynamic(this, &UShooterViewModel::HandleAmmoUIUpdated);
}

void UShooterViewModel::OnAnyGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UE_LOG(LogTemp, Log, TEXT("UShooterViewModel::OnAnyGameplayTagChange"))
	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("State.UI.UsingItem"));
	if (ASC)
	{
		UE_LOG(LogTemp, Log, TEXT("UShooterViewModel::OnAnyGameplayTagChanged::2"))
		bool bIsNowUsing = ASC->HasMatchingGameplayTag(RootTag);
		SetIsUsingItem(bIsNowUsing);
	}
}

void UShooterViewModel::HealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthPercent();
}

void UShooterViewModel::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthPercent();
}

void UShooterViewModel::UpdateHealthPercent()
{
	if (!ASC) return;

	float Health = ASC->GetNumericAttribute(UBpexAttributeSet::GetHealthAttribute());
	UE_LOG(LogTemp, Warning, TEXT("%f"), ASC->GetNumericAttribute(UBpexAttributeSet::GetHealthAttribute()));

	float MaxHealth = ASC->GetNumericAttribute(UBpexAttributeSet::GetMaxHealthAttribute());

	float NewPercent = MaxHealth > 0 ? Health / MaxHealth : 0;
	SetHealthPercent(NewPercent);
}


void UShooterViewModel::HandleItemUseStarted(float Duration)
{
	SetCurrentUseDuration(Duration);
}

void UShooterViewModel::HandleAmmoUIUpdated(int32 NewClipAmmo, int32 NewReserveAmmo)
{
	SetClipAmmo(NewClipAmmo);
	SetReserveAmmo(NewReserveAmmo);
}

void UShooterViewModel::HandleAmmoChanged(EAmmoType AmmoType, int32 NewCount)
{
	if (CombatComponent&&CombatComponent->GetCurrentAmmoType() == AmmoType)
	{
		SetReserveAmmo(NewCount);
	}
}

