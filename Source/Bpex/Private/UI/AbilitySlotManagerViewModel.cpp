// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySlotManagerViewModel.h"
#include "AbilitySystemComponent.h"
#include "UI/AbilitySlotViewModel.h"

void UAbilitySlotManagerViewModel::Initialize(
	UAbilitySystemComponent* InASC,
	const TArray<FAbilitySlotMapping>& InSlotMappings,
	bool bUseServerCooldown)
{
	if (!IsValid(InASC)) return;
	UWorld* World = InASC->GetWorld();
	// ─── 1. 建立 Tag → SlotVM 映射 ───
	FGameplayTagContainer CooldownTags;
	for (const FAbilitySlotMapping& Mapping : InSlotMappings)
	{
		if (!Mapping.CooldownTag.IsValid() || !IsValid(Mapping.SlotViewModel))
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid mapping entry, skipping"));
			continue;
		}
		// 设置每个 SlotVM 的 Tag 和 World
		Mapping.SlotViewModel->SetCooldownTag(Mapping.CooldownTag);
		Mapping.SlotViewModel->SetWorldContext(World);
		// 存入映射表
		TagToSlotMap.Add(Mapping.CooldownTag, Mapping.SlotViewModel);
		// 收集所有 Tag 给Listener
		CooldownTags.AddTag(Mapping.CooldownTag);
	}
	// ─── 2. 创建监听器 ───
	CooldownListener = NewObject<UCooldownListener>(this);
	CooldownListener->StartListening(InASC, CooldownTags, bUseServerCooldown);
	CooldownListener->OnCooldownBeginNative.AddUObject(
		this, &UAbilitySlotManagerViewModel::HandleCooldownBegin);
	CooldownListener->OnCooldownEndNative.AddUObject(
		this, &UAbilitySlotManagerViewModel::HandleCooldownEnd);
}

void UAbilitySlotManagerViewModel::HandleCooldownBegin(FGameplayTag Tag, float TimeRemaining, float Duration)
{
	if (UAbilitySlotViewModel* Found = TagToSlotMap.Find(Tag)->Get())
	{
		Found->StartCooldown(TimeRemaining, Duration);
	}
}

void UAbilitySlotManagerViewModel::HandleCooldownEnd(FGameplayTag Tag, float TimeRemaining, float Duration)
{
	if (UAbilitySlotViewModel* Found = TagToSlotMap.Find(Tag)->Get())
	{
		Found->EndCooldown();
	}
}
