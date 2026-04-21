// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BpexAbilitySystemComponent.h"

#include "BpexGameplayTags.h"
#include "AbilitySystem/Ability/BpexGameplayAbility.h"

void UBpexAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec,
                                                FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	OnEffectAssetTagsApplied.Broadcast(TagContainer);
}

void UBpexAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UBpexAbilitySystemComponent::EffectApplied);;
}

void UBpexAbilitySystemComponent::AbilityInputHeld(const FGameplayTag& InputTag)
{
}

void UBpexAbilitySystemComponent::AbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UBpexAbilitySystemComponent::AbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UBpexAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UBpexGameplayAbility* Ability = Cast<UBpexGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Ability->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}

void UBpexAbilitySystemComponent::DebugPrintTriggerMapping()
{
	UE_LOG(LogTemp, Warning, TEXT("=== Debug Trigger Mapping ==="));
	UE_LOG(LogTemp, Warning, TEXT("Is Server: %d"), GetOwner()->HasAuthority());
	UE_LOG(LogTemp, Warning, TEXT("Ability Count: %d"), 
		GetActivatableAbilities().Num());
	//打印所有技能名
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			UE_LOG(LogTemp, Warning, TEXT("  Ability: %s"), 
				*Spec.Ability->GetClass()->GetName());
		}
	}
	// ★ 直接检查映射表（ASC子类可以访问protected成员）
	UE_LOG(LogTemp, Warning, TEXT("EventTriggered Map Entries: %d"), 
		GameplayEventTriggeredAbilities.Num());
    
	for (auto& Pair : GameplayEventTriggeredAbilities)
	{
		UE_LOG(LogTemp, Warning, TEXT("  Tag: %s → %d abilities"), 
			*Pair.Key.ToString(), Pair.Value.Num());
        
		for (const FGameplayAbilitySpecHandle& Handle : Pair.Value)
		{
			FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
			if (Spec && Spec->Ability)
			{
				UE_LOG(LogTemp, Warning, TEXT("    → %s"), 
					*Spec->Ability->GetClass()->GetName());
			}
		}
	}
}

bool UBpexAbilitySystemComponent::GetCooldownRemainingForTag(FGameplayTagContainer CooldownTags, float& TimeRemaining,
	float& CooldownDuration)
{
	TimeRemaining = 0.f;
	CooldownDuration = 0.f;

	if (CooldownTags.Num() > 0)
	{
		// 构造一个查询器，查找包含任何指定 Tag 的 Gameplay Effect
		FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
        
		// 获取符合条件的 GE 的剩余时间和总持续时间
		TArray< TPair<float, float> > DurationAndTimeRemaining = GetActiveEffectsTimeRemainingAndDuration(Query);

		if (DurationAndTimeRemaining.Num() > 0)
		{
			// 如果同一个 Tag 触发了多次冷却（或重叠），找出剩余时间最长的那一个
			int32 BestIdx = 0;
			float LongestTime = DurationAndTimeRemaining[0].Key;
            
			for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
			{
				if (DurationAndTimeRemaining[Idx].Key > LongestTime)
				{
					LongestTime = DurationAndTimeRemaining[Idx].Key;
					BestIdx = Idx;
				}
			}

			// Key 是 TimeRemaining (剩余时间), Value 是 Duration (总时长)
			TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
			CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;
            
			return true; // 成功找到冷却中
		}
	}
	return false; // 当前没有该 Tag 的冷却
}

