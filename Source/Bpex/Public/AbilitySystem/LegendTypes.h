#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "LegendTypes.generated.h"

//========== 技能槽位 ==========
UENUM(BlueprintType)
enum class EAbilitySlotType : uint8
{
    Passive,
    Tactical,
    Ultimate
};

// ========== 护甲等级 ==========
UENUM(BlueprintType)
enum class EShieldTier : uint8
{
    None,       // 无甲
    Common,     // 白- 50HP
    Uncommon,   // 蓝 - 75HP → 可选增幅
    Rare,       // 紫 - 100HP → 可选增幅
    Epic,       // 金 - 100HP + 固定特效
    Legendary   // 红 - 125HP
};

// ========== 单个技能槽配置 ==========
USTRUCT(BlueprintType)
struct FAbilitySlotInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EAbilitySlotType SlotType = EAbilitySlotType::Passive;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<class ULegendGameplayAbility> AbilityClass;

    // 技能图标
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon;

    // 技能名称
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    // 技能描述
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText Description;

    // 授予后的 Ability Spec Handle (运行时)
    UPROPERTY(Transient)
    FGameplayAbilitySpecHandle GrantedHandle;
};

// ========== 增幅效果定义 ==========
USTRUCT(BlueprintType)
struct FShieldPerkInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag PerkTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText PerkName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText PerkDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> PerkIcon;

    // 增幅效果 (GAS GameplayEffect)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<class UGameplayEffect> PerkEffect;

    // 增幅关联的被动 Ability（如果需要更复杂的判断逻辑）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<class ULegendGameplayAbility> PerkAbility;

    // 解锁要求的最低护甲等级
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EShieldTier RequiredTier = EShieldTier::Uncommon;
};

USTRUCT(BlueprintType)
struct FShieldPerkEntry
{
    GENERATED_BODY()

    // 相当于原来 Map 的 Key
    UPROPERTY(BlueprintReadWrite)
    EShieldTier Tier;

    // 相当于原来 Map 的 Value
    UPROPERTY(BlueprintReadWrite)
    FShieldPerkInfo PerkInfo;

    // 写一个默认构造函数是个好习惯
    FShieldPerkEntry() {}
    FShieldPerkEntry(EShieldTier InTier, FShieldPerkInfo InInfo) 
        : Tier(InTier), PerkInfo(InInfo) {}

    // 重载 == 运算符，方便以后用 FindByKey 快速查找
    bool operator==(const EShieldTier& OtherTier) const
    {
        return Tier == OtherTier;
    }
};