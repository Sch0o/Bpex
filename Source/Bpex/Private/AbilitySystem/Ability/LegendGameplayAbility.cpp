#include "AbilitySystem/Ability/LegendGameplayAbility.h"
#include "AbilitySystem/LegendAbilityComponent.h"
#include "AbilitySystemComponent.h"
// ==================== Base ====================
ULegendGameplayAbility::ULegendGameplayAbility()
{
    // 默认设置
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
ULegendAbilityComponent* ULegendGameplayAbility::GetLegendAbilityComponent() const
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    return Avatar ? Avatar->FindComponentByClass<ULegendAbilityComponent>() : nullptr;
}
const FGameplayTagContainer* ULegendGameplayAbility::GetCooldownTags() const
{
    return &CooldownTags;
}
void ULegendGameplayAbility::GetCooldownTags(FGameplayTagContainer& OutTags) const
{
    OutTags = CooldownTags;
}
void ULegendGameplayAbility::ApplyCooldown(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (CooldownEffect)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffect, GetAbilityLevel());
        ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
    }
}
// ==================== Passive ====================
ULegendPassiveAbility::ULegendPassiveAbility()
{
    AbilitySlot = EAbilitySlotType::Passive;
    // 被动技能没有冷却
    // UI上不显示冷却
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}
void ULegendPassiveAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    // 子类重写来实现具体被动逻辑
    // 通常：Apply一个无限持续的 GE，或者 WaitGameplayEvent
}
// ==================== Tactical ====================
ULegendTacticalAbility::ULegendTacticalAbility()
{
    AbilitySlot = EAbilitySlotType::Tactical;
}
//==================== Ultimate ====================
ULegendUltimateAbility::ULegendUltimateAbility()
{
    AbilitySlot = EAbilitySlotType::Ultimate;
}
bool ULegendUltimateAbility::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }
    // 检查充能是否够
    AActor* Avatar = ActorInfo->AvatarActor.Get();
    if (Avatar)
    {
        ULegendAbilityComponent* LegendComp = Avatar->FindComponentByClass<ULegendAbilityComponent>();
        if (LegendComp)
        {
            return LegendComp->GetUltimateChargePercent() >= RequiredChargePercent;
        }
    }
    return false;
}