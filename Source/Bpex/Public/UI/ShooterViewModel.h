#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Weapon/AmmoTypes.h"
#include "GameplayTags.h"
#include "ShooterViewModel.generated.h"


class UCooldownListener;
class UCombatComponent;
class ULegendAbilityComponent;
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

	UFUNCTION(BlueprintCallable)
	void InitializeCombat(UCombatComponent* InCombat);
	
	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsUsingItem() const { return bIsUsingItem; }

	UFUNCTION(BlueprintCallable)
	void SetIsUsingItem(bool bNewState);

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetCurrentUseDuration() const { return CurrentUseDuration; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentUseDuration(float NewDuration);
	
	UFUNCTION(BlueprintCallable)
	void InitializeViewModel(APlayerController* PC);

protected:
	UPROPERTY()
	UAbilitySystemComponent* ASC;

	UPROPERTY()
	UInventoryComponent* InventoryComponent;

	UPROPERTY()
	ULegendAbilityComponent* LegendAbilityComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UCombatComponent> CombatComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UCooldownListener> CooldownListener = nullptr;

	void OnAnyGameplayTagChanged(const FGameplayTag Tag, int32 NewCount);

	void HealthChanged(const FOnAttributeChangeData& Data);

	void MaxHealthChanged(const FOnAttributeChangeData& Data);

	void UpdateHealthPercent();

	UFUNCTION()
	void HandleItemUseStarted(float Duration);

	UFUNCTION()
	void HandleAmmoUIUpdated(int32 NewClipAmmo, int32 NewReserveAmmo);

	UFUNCTION()
	void HandleAmmoChanged(EAmmoType AmmoType, int32 NewCount);

	UFUNCTION()
	void HandleCoolDownBegin(FGameplayTag Tag, float TimeRemaining, float Duration);

	UFUNCTION()
	void HandleCoolDownEnd(FGameplayTag Tag, float TimeRemaining, float Duration);
	
	FTimerHandle CooldownTimerHandle;
	
	UFUNCTION(Category="Init")
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

private:
	
	FGameplayTag TacticalCooldownTag;
	
	FGameplayTag UltimateCooldownTag;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetHealthPercent, Getter=GetHealthPercent,
		meta=(AllowPrivateAccess=true))
	float HealthPercent = 1.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetIsUsingItem, Getter=GetIsUsingItem,
		meta=(AllowPrivateAccess=true))
	bool bIsUsingItem = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetCurrentUseDuration, Getter=GetCurrentUseDuration,
		meta=(AllowPrivateAccess=true))
	float CurrentUseDuration = 0.0f;
	
	UPROPERTY()
	APlayerController* PlayerController;
};


