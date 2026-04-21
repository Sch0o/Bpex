// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "LegendTypes.h"
#include "LegendAbilityComponent.generated.h"
class UBpexAbilitySystemComponent;
class ULegendDataAsset;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilitySlotChanged, EAbilitySlotType, SlotType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerkSelected, EShieldTier, Tier, FShieldPerkInfo, Perk);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldTierChanged, EShieldTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerkSelectionRequired);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API ULegendAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    ULegendAbilityComponent();
    
    void InitAbilitySystem(UAbilitySystemComponent* InASC);
    
    /** 用DataAsset 初始化角色所有技能 */
    UFUNCTION(BlueprintCallable, Category = "Legend")
    void InitializeWithLegendData(const ULegendDataAsset* InLegendData);
    
    /** 激活指定槽位的技能 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    bool ActivateAbilityBySlot(EAbilitySlotType Slot);
    /** 取消指定槽位的技能 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    void CancelAbilityBySlot(EAbilitySlotType Slot);
    /** 获取技能冷却剩余时间 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    float GetCooldownRemainingBySlot(EAbilitySlotType Slot) const;
    /** 获取技能冷却百分比 (0~1) */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    float GetCooldownPercentBySlot(EAbilitySlotType Slot) const;
    /** 检查技能是否就绪 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    bool IsAbilityReady(EAbilitySlotType Slot) const;
    /** 获取技能信息 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ability")
    FAbilitySlotInfo GetAbilitySlotInfo(EAbilitySlotType Slot) const;
    
    /** 获取大招充能百分比 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ultimate")
    float GetUltimateChargePercent() const { return UltimateChargePercent; }
    /** 增加大招充能 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ultimate")
    void AddUltimateCharge(float Amount);
    /** 大招是否已充满 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Ultimate")
    bool IsUltimateReady() const { return UltimateChargePercent >= 1.0f; }
    //=============================================================
    // 护甲增幅
    //=============================================================
    /** 获取当前可选的增幅列表 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Perk")
    TArray<FShieldPerkInfo> GetAvailablePerks(EShieldTier Tier) const;
    /** 选择增幅 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Perk")
    bool SelectPerk(EShieldTier Tier, int32 PerkIndex);
    /** 获取已选择的增幅 */
    UFUNCTION(BlueprintCallable, Category = "Legend|Perk")
    bool GetActivePerk(EShieldTier Tier, FShieldPerkInfo& OutPerk) const;
    /** 清除增幅（降级护甲时） */
    UFUNCTION(BlueprintCallable, Category = "Legend|Perk")
    void ClearPerksAboveTier(EShieldTier Tier);
    //=============================================================
    // 护甲等级
    //=============================================================UFUNCTION(BlueprintCallable, Category = "Legend|Shield")
    void SetShieldTier(EShieldTier NewTier);
    UFUNCTION(BlueprintCallable, Category = "Legend|Shield")
    EShieldTier GetShieldTier() const { return CurrentShieldTier; }
    //=============================================================
    // 委托
    //=============================================================UPROPERTY(BlueprintAssignable, Category = "Legend|Events")
    FOnAbilitySlotChanged OnAbilitySlotChanged;
    UPROPERTY(BlueprintAssignable, Category = "Legend|Events")
    FOnPerkSelected OnPerkSelected;
    UPROPERTY(BlueprintAssignable, Category = "Legend|Events")
    FOnShieldTierChanged OnShieldTierChanged;
    /** 需要玩家选择增幅时广播（弹出UI） */
    UPROPERTY(BlueprintAssignable, Category = "Legend|Events")
    FOnPerkSelectionRequired OnPerkSelectionRequired;
protected:
    virtual void BeginPlay() override;
    //=============================================================
    // 内部数据
    //=============================================================UPROPERTY()
    TObjectPtr<const ULegendDataAsset> LegendData;
    UPROPERTY()
    TObjectPtr<UBpexAbilitySystemComponent> ASC;
    // 技能槽运行时数据
    UPROPERTY()
    TMap<EAbilitySlotType, FAbilitySlotInfo> AbilitySlots;
    // 当前护甲等级
    UPROPERTY(ReplicatedUsing = OnRep_ShieldTier)
    EShieldTier CurrentShieldTier = EShieldTier::None;
    // 已选增幅
    UPROPERTY(Replicated)
    TArray<FShieldPerkEntry> ActivePerks;
    
    // 已应用的增幅 Effect Handle（用于移除）
    TMap<EShieldTier, FActiveGameplayEffectHandle> ActivePerkEffectHandles;
    
    
    // 大招充能
    UPROPERTY(Replicated)
    float UltimateChargePercent = 0.f;
    UPROPERTY(EditDefaultsOnly, Category = "Ultimate")
    float UltimateAutoChargeRate = 0.01f; // 每秒自动充能1%
    //=============================================================
    // 网络同步
    //=============================================================
    
    UFUNCTION()
    void OnRep_ShieldTier();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
    void GrantAbility(FAbilitySlotInfo& SlotInfo);
    void RemoveAbility(FAbilitySlotInfo& SlotInfo);
    void ApplyPerkEffect(const FShieldPerkInfo& Perk, EShieldTier Tier);
    void RemovePerkEffect(EShieldTier Tier);
};
