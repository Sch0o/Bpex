// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LegendTypes.h"
#include "LegendDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class BPEX_API ULegendDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// ========== 角色信息 ==========
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend Info")
	FText LegendName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Legend Info")
	TSoftObjectPtr<UTexture2D> LegendPortrait;
	// ========== 技能配置 ==========
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FAbilitySlotInfo PassiveAbility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FAbilitySlotInfo TacticalAbility;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FAbilitySlotInfo UltimateAbility;
	// ========== 该角色可用的增幅选项 ==========
	//蓝甲时可选的增幅（选1个）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Perks")
	TArray<FShieldPerkInfo> BlueTierPerks;
	// 紫甲时可选的增幅（选1个）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shield Perks")
	TArray<FShieldPerkInfo> PurpleTierPerks;
	//========== 初始属性 ==========
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;
	
	// ========== 辅助函数 ==========
	const FAbilitySlotInfo& GetAbilityBySlot(EAbilitySlotType Slot) const
	{
		switch (Slot)
		{
		case EAbilitySlotType::Tactical: return TacticalAbility;
		case EAbilitySlotType::Ultimate: return UltimateAbility;
		default: return PassiveAbility;
		}
	}
};
