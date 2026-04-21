// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "ShooterViewModel.generated.h"

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

	//初始化技能系统
	UFUNCTION(BlueprintCallable, Category="Ability")
	void InitializeLegendAbility(ULegendAbilityComponent* InLAC);

	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsUsingItem() const { return bIsUsingItem; }

	UFUNCTION(BlueprintCallable)
	void SetIsUsingItem(bool bNewState);

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetCurrentUseDuration() const { return CurrentUseDuration; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentUseDuration(float NewDuration);
	
	
	UFUNCTION(BlueprintPure,FieldNotify)
	float GetTacticalCooldownPercent() const { return TacticalCooldownPercent; }
	UFUNCTION(BlueprintCallable)
	void SetTacticalCooldownPercent(float NewPercent);
	
	/** 冷却剩余秒数 */
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetTacticalCooldownRemaining() const { return TacticalCooldownRemaining; }
	UFUNCTION(BlueprintCallable)
	void SetTacticalCooldownRemaining(float NewRemaining);
	
	/** 战术技能是否就绪 */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsTacticalReady() const { return bIsTacticalReady; }
	UFUNCTION(BlueprintCallable)
	void SetIsTacticalReady(bool bNewReady);
	/**冷却剩余文本显示（如"12s"） */
	UFUNCTION(BlueprintPure)
	FText GetTacticalCooldownText() const;
	// ----- 大招充能 -----
	/** 充能百分比 0~1 */
	UFUNCTION(BlueprintPure, FieldNotify)
	float GetUltimateChargePercent() const { return UltimateChargePercent; }
	UFUNCTION(BlueprintCallable)
	void SetUltimateChargePercent(float NewPercent);
	/** 大招是否就绪 */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsUltimateReady() const { return bIsUltimateReady; }
	UFUNCTION(BlueprintCallable)
	void SetIsUltimateReady(bool bNewReady);

	UFUNCTION(BlueprintPure)
	FText GetUltimateChargeText() const;
	// ----- 技能图标（UI绑定用） -----
	UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* GetTacticalIcon() const { return TacticalIcon; }
	UFUNCTION(BlueprintCallable)
	void SetTacticalIcon(UTexture2D* NewIcon);UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* GetUltimateIcon() const { return UltimateIcon; }
	UFUNCTION(BlueprintCallable)
	void SetUltimateIcon(UTexture2D* NewIcon);

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
	
	//技能
	UPROPERTY()
	ULegendAbilityComponent* LegendAbilityComponent;
	
	UFUNCTION()
	void UpdateAbilityCooldowns();
	
	FTimerHandle CooldownTimerHandle;

private:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetHealthPercent, Getter=GetHealthPercent,
		meta=(AllowPrivateAccess=true))
	float HealthPercent = 1.0f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetClipAmmo, Getter=GetClipAmmo, meta=(AllowPrivateAccess=true))
	int32 ClipAmmo;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetReserveAmmo, Getter=GetReserveAmmo,
		meta=(AllowPrivateAccess=true))
	int32 ReserveAmmo;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetIsUsingItem, Getter=GetIsUsingItem,
		meta=(AllowPrivateAccess=true))
	bool bIsUsingItem = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter=SetCurrentUseDuration, Getter=GetCurrentUseDuration,
		meta=(AllowPrivateAccess=true))
	float CurrentUseDuration = 0.0f;

	//战术技能冷却进度
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetTacticalCooldownPercent, Getter = GetTacticalCooldownPercent,
		meta = (AllowPrivateAccess = true))
	float TacticalCooldownPercent = 0.f;

	//战术技能冷却剩余秒数
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetTacticalCooldownRemaining,
		Getter = GetTacticalCooldownRemaining, meta = (AllowPrivateAccess = true))
	float TacticalCooldownRemaining = 0.f;
	
	//战术技能是否就绪
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetIsTacticalReady,
		Getter = GetIsTacticalReady, meta = (AllowPrivateAccess = true))
	bool bIsTacticalReady = true;
	
	//绝招充能百分比
	UPROPERTY(BlueprintReadWrite, FieldNotify,Setter = SetUltimateChargePercent,
	   Getter = GetUltimateChargePercent, meta = (AllowPrivateAccess = true))
	float UltimateChargePercent = 0.f;
	
	//绝招是否就绪
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter = SetIsUltimateReady,
		Getter = GetIsUltimateReady, meta = (AllowPrivateAccess = true))
	bool bIsUltimateReady = false;
	
	//战术技能图标
	UPROPERTY(BlueprintReadWrite, FieldNotify,Setter = SetTacticalIcon,
		Getter = GetTacticalIcon, meta = (AllowPrivateAccess = true))
	TObjectPtr<UTexture2D> TacticalIcon = nullptr;
	
	//绝招技能图标
	UPROPERTY(BlueprintReadWrite, FieldNotify,
	   Setter = SetUltimateIcon,
	   Getter = GetUltimateIcon,
	   meta = (AllowPrivateAccess = true))
	TObjectPtr<UTexture2D> UltimateIcon = nullptr;
	
};
