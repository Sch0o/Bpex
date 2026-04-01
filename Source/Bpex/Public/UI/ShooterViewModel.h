// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ShooterViewModel.generated.h"

class UInventoryComponent;
struct FGameplayTag;
struct FOnAttributeChangeData;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class BPEX_API UShooterViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	float GetHealthPercent() const { return HealthPercent; }
	
	UFUNCTION(BlueprintPure)
	FText GetClipAmmoText() const { return FText::AsNumber(ClipAmmo); }
	
	UFUNCTION(BlueprintPure)
	FText GetReserveAmmoText() const { return FText::AsNumber(ReserveAmmo); }
	
	UFUNCTION(BlueprintCallable)
	int32 GetClipAmmo() const { return ClipAmmo; }
	
	UFUNCTION(BlueprintCallable)
	int32 GetReserveAmmo() const { return ReserveAmmo; }
	
	UFUNCTION(BlueprintCallable)
	void SetHealthPercent(float NewPercent);
	
	UFUNCTION(BlueprintCallable)
	void SetReserveAmmo(int32 NewMaxClipAmmo);
	
	UFUNCTION(BlueprintCallable)
	void SetClipAmmo(int32 NewClipAmmo);
	
	UFUNCTION(BlueprintCallable)
	void InitializeASC(UAbilitySystemComponent* InASC);
	
	UFUNCTION(BlueprintCallable)
	void InitializeInventory(UInventoryComponent* InIC);
	
	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsUsingItem() const { return bIsUsingItem; }
    
	UFUNCTION(BlueprintCallable)
	void SetIsUsingItem(bool bNewState);

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetCurrentUseDuration() const { return CurrentUseDuration; }
    
	UFUNCTION(BlueprintCallable)
	void SetCurrentUseDuration(float NewDuration);

protected:
	UPROPERTY()
	UAbilitySystemComponent* ASC;
	
	UPROPERTY()
	UInventoryComponent* InventoryComponent;
	
	void OnAnyGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);

	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	
	void UpdateHealthPercent();
	
	void ClipAmmoChanged(const FOnAttributeChangeData& Data);
	void UpdateClipAmmo();
	void ReserveAmmoChanged(const FOnAttributeChangeData& Data);
	void UpdateReserveAmmo();
	
	UFUNCTION()
	void HandleItemUseStarted(float Duration);

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetHealthPercent, Getter=GetHealthPercent,
		meta=(AllowPrivateAccess=true))
	float HealthPercent = 1.0f;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetClipAmmo,Getter=GetClipAmmo,meta=(AllowPrivateAccess=true))
	int32 ClipAmmo;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetReserveAmmo,Getter=GetReserveAmmo,meta=(AllowPrivateAccess=true))
	int32 ReserveAmmo;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetIsUsingItem, Getter=GetIsUsingItem, meta=(AllowPrivateAccess=true))
	bool bIsUsingItem = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetCurrentUseDuration, Getter=GetCurrentUseDuration, meta=(AllowPrivateAccess=true))
	float CurrentUseDuration = 0.0f;
	
};
