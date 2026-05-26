// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShooterViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BpexGameplayTags.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "Weapon/CombatComponent.h"
#include "AbilitySystem/LegendAbilityComponent.h"
#include "AbilitySystem/Ability/CooldownListener.h"
#include "InventorySystem/InventoryComponent.h"


void UShooterViewModel::SetHealthPercent(float NewPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, NewPercent);
}

void UShooterViewModel::SetReserveAmmo(int32 NewReserveAmmo)
{
	UE_MVVM_SET_PROPERTY_VALUE(ReserveAmmo, NewReserveAmmo);
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

void UShooterViewModel::SetTacticalCooldownPercent(float NewPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(TacticalCooldownPercent, NewPercent);
}

void UShooterViewModel::SetTacticalCooldownRemaining(float NewRemaining)
{
	UE_MVVM_SET_PROPERTY_VALUE(TacticalCooldownRemaining, NewRemaining);
}

void UShooterViewModel::SetIsTacticalReady(bool bNewReady)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsTacticalReady, bNewReady);
}

FText UShooterViewModel::GetTacticalCooldownText() const
{
	return FText();
}

void UShooterViewModel::SetUltimateChargePercent(float NewPercent)
{
	UE_MVVM_SET_PROPERTY_VALUE(UltimateChargePercent, NewPercent);
}

void UShooterViewModel::SetIsUltimateReady(bool bNewReady)
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsUltimateReady, bNewReady);
}

FText UShooterViewModel::GetUltimateChargeText() const
{
	return FText();
}

void UShooterViewModel::SetTacticalIcon(UTexture2D* NewIcon)
{
	UE_MVVM_SET_PROPERTY_VALUE(TacticalIcon, NewIcon);
}

void UShooterViewModel::SetUltimateIcon(UTexture2D* NewIcon)
{
	UE_MVVM_SET_PROPERTY_VALUE(UltimateIcon, NewIcon);
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
	
	
	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(FBpexGameplayTags::Get().Cooldown_Ability_Tactical);
	CooldownTags.AddTag(FBpexGameplayTags::Get().Cooldown_Ability_Ultimate);
	
	CooldownListener = NewObject<UCooldownListener>(this);
	CooldownListener->StartListening(ASC,CooldownTags,true);
	
}

void UShooterViewModel::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Log, TEXT("UShooterViewModel::HandlePawnChanged:: NewPawn is null"));
		return;
	}
	if (ULegendAbilityComponent* AbilityComp = NewPawn->FindComponentByClass<ULegendAbilityComponent>())
	{
		InitializeLegendAbility(AbilityComp);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("UShooterViewModel::HandlePawnChanged:: LegendAbilityComponent is null"));
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

void UShooterViewModel::InitializeLegendAbility(ULegendAbilityComponent* InLAC)
{
	if (!InLAC)
	{
		UE_LOG(LogTemp, Error, TEXT("UShooterViewModel::InitializeLegendAbility:: LegendAbilityComponent is null"))
		return;
	}

	// LegendAbilityComponent = InLAC;
	// //战术技能
	// FAbilitySlotInfo TacticalInfo = LegendAbilityComponent->GetAbilitySlotInfo(
	// 	EAbilitySlotType::Tactical);
	// if (UTexture2D* Tex = TacticalInfo.Icon.LoadSynchronous())
	// {
	// 	SetTacticalIcon(Tex);
	// }
	// //终极技能
	// FAbilitySlotInfo UltInfo = LegendAbilityComponent->GetAbilitySlotInfo(
	// 	EAbilitySlotType::Ultimate);
	// if (UTexture2D* Tex = UltInfo.Icon.LoadSynchronous())
	// {
	// 	SetUltimateIcon(Tex);
	// }
	// // 定时器，每帧刷新冷却/充能
	// if (UWorld* World = GetWorld())
	// {
	// 	World->GetTimerManager().SetTimer(
	// 		CooldownTimerHandle,
	// 		this,
	// 		&UShooterViewModel::UpdateAbilityCooldowns,
	// 		0.05f, 
	// 		true 
	// 	);
	// }
	
	// 初始状态
	SetIsTacticalReady(true);
	SetTacticalCooldownPercent(1.f);
	SetUltimateChargePercent(0.f);
	SetIsUltimateReady(false);
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


void UShooterViewModel::HandleCoolDownEnd(FGameplayTag Tag, float TimeRemaining, float Duration)
{
	if (Tag.MatchesTagExact())
}

void UShooterViewModel::HandleCoolDownBegin(FGameplayTag Tag, float TimeRemaining, float Duration)
{
	
}
