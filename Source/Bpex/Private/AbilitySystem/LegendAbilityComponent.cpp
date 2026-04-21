#include "AbilitySystem/LegendAbilityComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "AbilitySystem/Ability/LegendGameplayAbility.h"
#include "AbilitySystem/LegendDataAsset.h"
#include "Net/UnrealNetwork.h"

ULegendAbilityComponent::ULegendAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void ULegendAbilityComponent::InitAbilitySystem(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		UE_LOG(LogTemp, Error, TEXT("ULegendAbilityComponent::InitAbilitySystem::InASC is invalid"));
	}
	ASC = Cast<UBpexAbilitySystemComponent>(InASC);

	if (!ASC)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("ULegendAbilityComponent::InitAbilitySystem::InASC is not UBpexAbilitySystemComponent"));
	}
}

void ULegendAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULegendAbilityComponent::InitializeWithLegendData(const ULegendDataAsset* InLegendData)
{
	if (!InLegendData || !ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeWithLegendData: 无效的数据或ASC"));
		return;
	}

	LegendData = InLegendData;

	// 清理旧的
	for (auto& Pair : AbilitySlots)
	{
		RemoveAbility(Pair.Value);
	}
	AbilitySlots.Empty();

	// ===== 注册被动技能 =====
	FAbilitySlotInfo PassiveSlot = LegendData->PassiveAbility;
	PassiveSlot.SlotType = EAbilitySlotType::Passive;
	GrantAbility(PassiveSlot);
	AbilitySlots.Add(EAbilitySlotType::Passive, PassiveSlot);

	// ===== 注册战术技能 =====
	FAbilitySlotInfo TacticalSlot = LegendData->TacticalAbility;
	TacticalSlot.SlotType = EAbilitySlotType::Tactical;
	GrantAbility(TacticalSlot);
	AbilitySlots.Add(EAbilitySlotType::Tactical, TacticalSlot);

	// ===== 注册大招 =====
	FAbilitySlotInfo UltimateSlot = LegendData->UltimateAbility;
	UltimateSlot.SlotType = EAbilitySlotType::Ultimate;
	GrantAbility(UltimateSlot);
	AbilitySlots.Add(EAbilitySlotType::Ultimate, UltimateSlot);

	// 应用默认属性
	if (LegendData->DefaultAttributes)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(
			LegendData->DefaultAttributes, 1, Context);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}

	UE_LOG(LogTemp, Log, TEXT("Legend [%s] 技能初始化完成"), *LegendData->LegendName.ToString());
}

//=============================================================
// 授予/移除技能
//=============================================================
void ULegendAbilityComponent::GrantAbility(FAbilitySlotInfo& SlotInfo)
{
	if (!ASC || !SlotInfo.AbilityClass) return;

	FGameplayAbilitySpec Spec(SlotInfo.AbilityClass, 1, INDEX_NONE, GetOwner());

	// 设置 InputID（用于绑定输入）
	Spec.InputID = static_cast<int32>(SlotInfo.SlotType);

	SlotInfo.GrantedHandle = ASC->GiveAbility(Spec);

	// 被动技能：立即激活
	if (SlotInfo.SlotType == EAbilitySlotType::Passive)
	{
		ASC->TryActivateAbility(SlotInfo.GrantedHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("授予技能: %s [Slot: %d]"),
	       *SlotInfo.AbilityClass->GetName(),
	       static_cast<int32>(SlotInfo.SlotType));
}

void ULegendAbilityComponent::RemoveAbility(FAbilitySlotInfo& SlotInfo)
{
	if (!ASC || !SlotInfo.GrantedHandle.IsValid()) return;

	ASC->ClearAbility(SlotInfo.GrantedHandle);
	SlotInfo.GrantedHandle = FGameplayAbilitySpecHandle();
}

//=============================================================
// 技能操作
//=============================================================
bool ULegendAbilityComponent::ActivateAbilityBySlot(EAbilitySlotType Slot)
{
	if (!ASC) return false;

	// 大招需要检查充能
	if (Slot == EAbilitySlotType::Ultimate && !IsUltimateReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("大招未充满: %.1f%%"), UltimateChargePercent * 100.f);
		return false;
	}

	const FAbilitySlotInfo* SlotInfo = AbilitySlots.Find(Slot);
	if (!SlotInfo || !SlotInfo->GrantedHandle.IsValid()) return false;

	bool bSuccess = ASC->TryActivateAbility(SlotInfo->GrantedHandle);

	if (bSuccess && Slot == EAbilitySlotType::Ultimate)
	{
		// 使用大招后清空充能
		UltimateChargePercent = 0.f;
	}

	return bSuccess;
}

void ULegendAbilityComponent::CancelAbilityBySlot(EAbilitySlotType Slot)
{
	if (!ASC) return;

	const FAbilitySlotInfo* SlotInfo = AbilitySlots.Find(Slot);
	if (!SlotInfo) return;

	ASC->CancelAbilityHandle(SlotInfo->GrantedHandle);
}

float ULegendAbilityComponent::GetCooldownRemainingBySlot(EAbilitySlotType Slot) const
{
	if (!ASC) return 0.f;

	const FAbilitySlotInfo* SlotInfo = AbilitySlots.Find(Slot);
	if (!SlotInfo || !SlotInfo->AbilityClass) return 0.f;

	const ULegendGameplayAbility* AbilityCDO = SlotInfo->AbilityClass.GetDefaultObject();
	if (!AbilityCDO) return 0.f;

	FGameplayTagContainer CooldownTags;
	AbilityCDO->GetCooldownTags(CooldownTags);

	float TimeRemaining = 0.f;
	float Duration = 0.f;
	ASC->GetCooldownRemainingForTag(CooldownTags, TimeRemaining, Duration);

	return TimeRemaining;
}

float ULegendAbilityComponent::GetCooldownPercentBySlot(EAbilitySlotType Slot) const
{
	// 类似上方，返回 TimeRemaining / Duration
	if (!ASC) return 0.f;

	const FAbilitySlotInfo* SlotInfo = AbilitySlots.Find(Slot);
	if (!SlotInfo || !SlotInfo->AbilityClass) return 0.f;

	const ULegendGameplayAbility* CDO = SlotInfo->AbilityClass.GetDefaultObject();
	FGameplayTagContainer CooldownTags;
	CDO->GetCooldownTags(CooldownTags);

	float TimeRemaining = 0.f, Duration = 0.f;
	ASC->GetCooldownRemainingForTag(CooldownTags, TimeRemaining, Duration);

	return Duration > 0.f ? TimeRemaining / Duration : 0.f;
}

bool ULegendAbilityComponent::IsAbilityReady(EAbilitySlotType Slot) const
{
	return GetCooldownRemainingBySlot(Slot) <= 0.f;
}

FAbilitySlotInfo ULegendAbilityComponent::GetAbilitySlotInfo(EAbilitySlotType Slot) const
{
	const FAbilitySlotInfo* Found = AbilitySlots.Find(Slot);
	return Found ? *Found : FAbilitySlotInfo();
}

//=============================================================
// 大招充能
//=============================================================
void ULegendAbilityComponent::AddUltimateCharge(float Amount)
{
	UltimateChargePercent = FMath::Clamp(UltimateChargePercent + Amount, 0.f, 1.f);
}

//=============================================================
// 护甲增幅系统
//=============================================================
void ULegendAbilityComponent::SetShieldTier(EShieldTier NewTier)
{
	// EShieldTier OldTier = CurrentShieldTier;
	// CurrentShieldTier = NewTier;
	//
	// // 降级时清除高等级增幅
	// if (NewTier < OldTier)
	// {
	//     ClearPerksAboveTier(NewTier);
	// }
	//
	// // 升到蓝甲或紫甲时，触发增幅选择
	// if (NewTier >= EShieldTier::Uncommon && OldTier < NewTier)
	// {
	//     // 检查该等级是否有增幅需要选择
	//     TArray<FShieldPerkInfo> AvailablePerks = GetAvailablePerks(NewTier);
	//     if (AvailablePerks.Num() > 0 && !ActivePerks.Contains(NewTier))
	//     {
	//         // 广播事件：通知UI弹出增幅选择界面
	//         OnPerkSelectionRequired.Broadcast();
	//     }
	// }
	//
	// OnShieldTierChanged.Broadcast(NewTier);
}

TArray<FShieldPerkInfo> ULegendAbilityComponent::GetAvailablePerks(EShieldTier Tier) const
{
	if (!LegendData) return {};

	switch (Tier)
	{
	case EShieldTier::Uncommon:
		return LegendData->BlueTierPerks;
	case EShieldTier::Rare:
		return LegendData->PurpleTierPerks;
	default:
		return {};
	}
}

bool ULegendAbilityComponent::SelectPerk(EShieldTier Tier, int32 PerkIndex)
{
	TArray<FShieldPerkInfo> Available = GetAvailablePerks(Tier);

	if (!Available.IsValidIndex(PerkIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("无效的增幅索引: %d"), PerkIndex);
		return false;
	}

	// 如果已有这个等级的增幅，先移除
	RemovePerkEffect(Tier);

	// 应用新增幅
	FShieldPerkInfo SelectedPerk = Available[PerkIndex];
	ApplyPerkEffect(SelectedPerk, Tier);
	// ActivePerks.Add(Tier, SelectedPerk);

	OnPerkSelected.Broadcast(Tier, SelectedPerk);

	UE_LOG(LogTemp, Log, TEXT("选择增幅: %s (Tier: %d)"),
	       *SelectedPerk.PerkName.ToString(), static_cast<int32>(Tier));

	return true;
}

bool ULegendAbilityComponent::GetActivePerk(EShieldTier Tier, FShieldPerkInfo& OutPerk) const
{
	// const FShieldPerkInfo* Found = ActivePerks.Find(Tier);
	// if (Found)
	// {
	//     OutPerk = *Found;
	//     return true;
	// }
	// return false;
	return true;
}

void ULegendAbilityComponent::ClearPerksAboveTier(EShieldTier Tier)
{
	// TArray<EShieldTier> TiersToRemove;
	//
	// for (auto& Pair : ActivePerks)
	// {
	//     if (Pair.Key > Tier)
	//     {
	//         TiersToRemove.Add(Pair.Key);
	//     }
	// }
	//
	// for (EShieldTier T : TiersToRemove)
	// {
	//     RemovePerkEffect(T);ActivePerks.Remove(T);
	// }
}

void ULegendAbilityComponent::ApplyPerkEffect(const FShieldPerkInfo& Perk, EShieldTier Tier)
{
	if (!ASC) return;

	// 应用 GameplayEffect
	if (Perk.PerkEffect)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Perk.PerkEffect, 1, Context);
		FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		ActivePerkEffectHandles.Add(Tier, Handle);
	}

	// 授予 Perk Ability（如果有）
	if (Perk.PerkAbility)
	{
		FGameplayAbilitySpec AbilitySpec(Perk.PerkAbility, 1, INDEX_NONE, GetOwner());
		ASC->GiveAbility(AbilitySpec);
	}
}

void ULegendAbilityComponent::RemovePerkEffect(EShieldTier Tier)
{
	if (!ASC) return;

	FActiveGameplayEffectHandle* Handle = ActivePerkEffectHandles.Find(Tier);
	if (Handle && Handle->IsValid())
	{
		ASC->RemoveActiveGameplayEffect(*Handle);
		ActivePerkEffectHandles.Remove(Tier);
	}
}

//=============================================================
// 网络
//=============================================================
void ULegendAbilityComponent::OnRep_ShieldTier()
{
	OnShieldTierChanged.Broadcast(CurrentShieldTier);
}

void ULegendAbilityComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULegendAbilityComponent, CurrentShieldTier);
	DOREPLIFETIME(ULegendAbilityComponent, UltimateChargePercent);
}
