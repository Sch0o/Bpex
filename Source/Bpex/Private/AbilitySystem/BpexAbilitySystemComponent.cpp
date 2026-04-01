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
