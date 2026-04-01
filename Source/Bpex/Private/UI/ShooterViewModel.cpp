// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "InventorySystem/InventoryComponent.h"


void UShooterViewModel::SetHealthPercent(float NewPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, NewPercent);
}

void UShooterViewModel::SetReserveAmmo(int32 NewReserveAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(ReserveAmmo, NewReserveAmmo);
}

void UShooterViewModel::InitializeASC(UAbilitySystemComponent* InASC)
{
	ASC = InASC;
	if (!ASC)
	{
		UE_LOG(LogTemp,Error,TEXT("ViewModel Init fail:ASC is null"))
		return;
	} 
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetHealthAttribute()).AddUObject(
		this, &UShooterViewModel::HealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetMaxHealthAttribute()).AddUObject(
		this, &UShooterViewModel::MaxHealthChanged);
	
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetClipAmmoAttribute()).AddUObject(
	this, &UShooterViewModel::ClipAmmoChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UBpexAttributeSet::GetReserveAmmoAttribute()).AddUObject(
		this, &UShooterViewModel::ReserveAmmoChanged);
	
	UpdateHealthPercent();
	UpdateClipAmmo();
	UpdateReserveAmmo();
	
	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("State.UI.UsingItem"));
	ASC->RegisterGameplayTagEvent(RootTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UShooterViewModel::OnAnyGameplayTagChanged);
}

void UShooterViewModel::InitializeInventory(UInventoryComponent* InIC)
{
	InventoryComponent = InIC;
	if (!InIC)
	{
		UE_LOG(LogTemp,Error,TEXT("ViewModel Init fail:InventoryComponent is null"))
		return;
	}
	InIC->OnItemUseStarted.AddDynamic(this, &UShooterViewModel::HandleItemUseStarted);
}

void UShooterViewModel::SetIsUsingItem(bool bNewState)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsUsingItem, bNewState);
}

void UShooterViewModel::SetClipAmmo(int32 NewClipAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(ClipAmmo, NewClipAmmo);
}

void UShooterViewModel::SetCurrentUseDuration(float NewDuration)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentUseDuration, NewDuration);
}

void UShooterViewModel::OnAnyGameplayTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UE_LOG(LogTemp,Log,TEXT("UShooterViewModel::OnAnyGameplayTagChange"))
	FGameplayTag RootTag = FGameplayTag::RequestGameplayTag(FName("State.UI.UsingItem"));
	if (ASC)
	{
		UE_LOG(LogTemp,Log,TEXT("UShooterViewModel::OnAnyGameplayTagChanged::2"))
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
	UE_LOG(LogTemp,Warning,TEXT("%f"),ASC->GetNumericAttribute(UBpexAttributeSet::GetHealthAttribute()));

	float MaxHealth = ASC->GetNumericAttribute(UBpexAttributeSet::GetMaxHealthAttribute());

	float NewPercent = MaxHealth > 0 ? Health / MaxHealth : 0;
	SetHealthPercent(NewPercent);
}

void UShooterViewModel::ClipAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetClipAmmo(Data.NewValue);
}

void UShooterViewModel::UpdateClipAmmo()
{
	if (!ASC) return;
	SetClipAmmo(ASC->GetNumericAttribute(UBpexAttributeSet::GetClipAmmoAttribute()));
}

void UShooterViewModel::ReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetReserveAmmo(Data.NewValue);
}

void UShooterViewModel::UpdateReserveAmmo()
{
	if (!ASC) return;
	UE_LOG(LogTemp,Warning,TEXT("%f"),ASC->GetNumericAttribute(UBpexAttributeSet::GetReserveAmmoAttribute()));
	SetReserveAmmo(ASC->GetNumericAttribute(UBpexAttributeSet::GetReserveAmmoAttribute()));
}

void UShooterViewModel::HandleItemUseStarted(float Duration)
{
	SetCurrentUseDuration(Duration);
}
