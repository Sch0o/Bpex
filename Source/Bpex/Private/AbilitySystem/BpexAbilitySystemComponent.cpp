// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BpexAbilitySystemComponent.h"

#include "BpexGameplayTags.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/BpexAbilityTypes.h"
#include "AbilitySystem/Ability/BpexGameplayAbility.h"

void UBpexAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& EffectSpec,
                                                FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	OnEffectAssetTagsApplied.Broadcast(TagContainer);
}

UBpexAbilitySystemComponent::UBpexAbilitySystemComponent()
{
	InputPressedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UBpexAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UBpexAbilitySystemComponent::EffectApplied);;
}

void UBpexAbilitySystemComponent::AbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())return;
	
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability&&AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UBpexAbilitySystemComponent::AbilityInputReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
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

void UBpexAbilitySystemComponent::ProcessAbilityInput(float DeltaTime,const bool bGamePaused)
{
	if (HasMatchingGameplayTag(FBpexGameplayTags::Get().Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}
	
	//static 保证可以不用重复开辟内存，优化性能
	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	
	// Process all abilities that activate when the input is held.
	// 某些技能例如开火可能在held期间End，这时就需要继续激活
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UBpexGameplayAbility* AbilityCDO = Cast<UBpexGameplayAbility>(AbilitySpec->Ability);
				if (AbilityCDO && AbilityCDO->GetActivationPolicy() == EBpexAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}
	
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UBpexGameplayAbility* LyraAbilityCDO = Cast<UBpexGameplayAbility>(AbilitySpec->Ability);

					if (LyraAbilityCDO && LyraAbilityCDO->GetActivationPolicy() == EBpexAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}
	
	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}
	
	
	// Process all abilities that had their input released this frame.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}
	
	
	// Clear the cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	
}

void UBpexAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

