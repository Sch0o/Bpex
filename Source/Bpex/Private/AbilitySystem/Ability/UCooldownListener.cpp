// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/CooldownListener.h"
#include "AbilitySystemComponent.h"
void UCooldownListener::StartListening(
    UAbilitySystemComponent* InASC,
    const FGameplayTagContainer& InCooldownTags,
    bool bInUseServerCooldown)
{
    if (bIsListening)
    {
        StopListening();
    }
    ASC = InASC;
    CooldownTags = InCooldownTags;
    bUseServerCooldown = bInUseServerCooldown;
    if (!IsValid(ASC) || CooldownTags.Num() == 0)
    {
        return;
    }
    // 监听 GE 添加（检测CD开始）
    ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
        this, &UCooldownListener::OnActiveGameplayEffectAdded);
    // 监听每个 CooldownTag 的变化（检测CD结束）
    TArray<FGameplayTag> TagArray;
    CooldownTags.GetGameplayTagArray(TagArray);
    for (const FGameplayTag& Tag : TagArray)
    {
        ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
            .AddUObject(this, &UCooldownListener::OnCooldownTagChanged);
    }
    bIsListening = true;
}
void UCooldownListener::StopListening()
{
    if (!bIsListening) return;
    if (IsValid(ASC))
    {
        ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
        TArray<FGameplayTag> TagArray;
        CooldownTags.GetGameplayTagArray(TagArray);
        for (const FGameplayTag& Tag : TagArray)
        {
            ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
                .RemoveAll(this);
        }
    }
    // 清空委托
    OnCooldownBeginNative.Clear();
    OnCooldownEndNative.Clear();
    OnCooldownBeginDynamic.Clear();
    OnCooldownEndDynamic.Clear();
    
    ASC = nullptr;
    bIsListening = false;
}
void UCooldownListener::OnActiveGameplayEffectAdded(
    UAbilitySystemComponent* Target,
    const FGameplayEffectSpec& SpecApplied,
    FActiveGameplayEffectHandle ActiveHandle)
{
    FGameplayTagContainer AssetTags;
    SpecApplied.GetAllAssetTags(AssetTags);
    FGameplayTagContainer GrantedTags;
    SpecApplied.GetAllGrantedTags(GrantedTags);
    
    TArray<FGameplayTag> TagArray;
    CooldownTags.GetGameplayTagArray(TagArray);
    
    for (const FGameplayTag& CooldownTag : TagArray)
    {
        if (!AssetTags.HasTagExact(CooldownTag) && !GrantedTags.HasTagExact(CooldownTag))
        {
            continue;
        }
        float TimeRemaining = 0.f;
        float Duration = 0.f;
        // 修复原版Bug：用匹配到的CooldownTag查询，而不是固定取Index(0)
        FGameplayTagContainer QueryTags(CooldownTag);
        GetCooldownRemainingForTag(QueryTags, TimeRemaining, Duration);
        
        const bool bIsServer = ASC->GetOwnerRole() == ROLE_Authority;
        const bool bIsPredicted = SpecApplied.GetContext().GetAbilityInstance_NotReplicated() != nullptr;
        if (bIsServer)
        {
            BroadcastCooldownBegin(CooldownTag, TimeRemaining, Duration);
        }
        else if (!bUseServerCooldown && bIsPredicted)
        {
            // 客户端用本地预测CD
            BroadcastCooldownBegin(CooldownTag, TimeRemaining, Duration);
        }
        else if (bUseServerCooldown && !bIsPredicted)
        {
            // 客户端用服务器CD（收到服务器修正）
            BroadcastCooldownBegin(CooldownTag, TimeRemaining, Duration);
        }
        else if (bUseServerCooldown && bIsPredicted)
        {
            // 等待服务器确认，先占位
            BroadcastCooldownBegin(CooldownTag, -1.f, -1.f);
        }}
}
void UCooldownListener::OnCooldownTagChanged(
    const FGameplayTag CooldownTag, int32 NewCount)
{
    if (NewCount == 0)
    {
        BroadcastCooldownEnd(CooldownTag);
    }
}
bool UCooldownListener::GetCooldownRemainingForTag(
    const FGameplayTagContainer& InTags,
    float& OutTimeRemaining,
    float& OutDuration) const
{
    if (!IsValid(ASC) || InTags.Num() == 0)
        return false;
    OutTimeRemaining = 0.f;
    OutDuration = 0.f;
    FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(InTags);
    
    TArray<TPair<float, float>> Results =
        ASC->GetActiveEffectsTimeRemainingAndDuration(Query);
    if (Results.Num() == 0)
        return false;
    
    // 取剩余时间最长的
    int32 BestIdx = 0;
    float LongestTime = Results[0].Key;
    for (int32 i = 1; i < Results.Num(); ++i)
    {
        if (Results[i].Key > LongestTime)
        {
            LongestTime = Results[i].Key;
            BestIdx = i;
        }
    }
    
    OutTimeRemaining = Results[BestIdx].Key;
    OutDuration = Results[BestIdx].Value;
    return true;
}
void UCooldownListener::BroadcastCooldownBegin(
    FGameplayTag Tag, float TimeRemaining, float Duration)
{
    // 原生委托（C++高性能绑定）
    OnCooldownBeginNative.Broadcast(Tag, TimeRemaining, Duration);// 动态委托（蓝图/MVVM绑定）
    OnCooldownBeginDynamic.Broadcast(Tag, TimeRemaining, Duration);
}
void UCooldownListener::BroadcastCooldownEnd(FGameplayTag Tag)
{
    OnCooldownEndNative.Broadcast(Tag, -1.f, -1.f);
    OnCooldownEndDynamic.Broadcast(Tag, -1.f, -1.f);
}