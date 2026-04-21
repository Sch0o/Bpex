// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "LegendGameplayAbility.generated.h"

enum class EAbilitySlotType : uint8;
class ULegendAbilityComponent;

UCLASS()
class BPEX_API ULegendGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	ULegendGameplayAbility();
	// 技能所属槽位
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend")
	EAbilitySlotType AbilitySlot;
	// 冷却时间标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend")
	FGameplayTagContainer CooldownTags;
	//冷却时间 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend")
	TSubclassOf<UGameplayEffect> CooldownEffect;
	// 消耗 GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend")
	TSubclassOf<UGameplayEffect> CostEffect;
	// 获取冷却标签
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	void GetCooldownTags(FGameplayTagContainer& OutTags) const;
	// 应用冷却
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	                           const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// 获取 LegendAbilityComponent
	UFUNCTION(BlueprintCallable, Category = "Legend")
	ULegendAbilityComponent* GetLegendAbilityComponent() const;
};

// ========== 被动技能基类 ==========
UCLASS(Abstract)
class BPEX_API ULegendPassiveAbility : public ULegendGameplayAbility
{
	GENERATED_BODY()

public:
	ULegendPassiveAbility();
	// 被动技能激活后一直运行，通常用 WaitGameplayEvent 等待触发
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
};

// ========== 战术技能基类 ==========
UCLASS(Abstract)
class BPEX_API ULegendTacticalAbility : public ULegendGameplayAbility
{
	GENERATED_BODY()

public:
	ULegendTacticalAbility();
	// 默认CD
	UPROPERTY(EditDefaultsOnly, Category = "Tactical")
	float DefaultCooldownDuration = 15.0f;
	// 最大蓄力次数（部分角色有多段蓄力）
	UPROPERTY(EditDefaultsOnly, Category = "Tactical")
	int32 MaxCharges = 1;
};

// ========== 大招技能基类 ==========
UCLASS(Abstract)
class BPEX_API ULegendUltimateAbility : public ULegendGameplayAbility
{
	GENERATED_BODY()

public:
	ULegendUltimateAbility();
	// 需要的充能百分比（通常100%）
	UPROPERTY(EditDefaultsOnly, Category = "Ultimate")
	float RequiredChargePercent = 1.0f;
	// 重写CanActivateAbility，检查充能
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;
};
