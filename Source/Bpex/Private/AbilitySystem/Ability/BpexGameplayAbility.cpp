// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Ability/BpexGameplayAbility.h"
#include "AbilitySystemComponent.h"


void UBpexGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);
	
	if (ActivationPolicy == EBpexAbilityActivationPolicy::OnSpawn)
	{
		if (ActorInfo && ActorInfo->AbilitySystemComponent.Get())
		{
			if (ActorInfo->IsNetAuthority())
			{
				ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
			}
		}
	}
}
