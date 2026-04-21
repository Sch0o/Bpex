// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/Test/Test_UltimateAbility.h"
#include "AbilitySystemComponent.h"

UTest_UltimateAbility::UTest_UltimateAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UTest_UltimateAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                        const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_AutoHeal::ActivateAbility::No ASC"));
	}

	if (!HealEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGA_AutoHeal::ActivateAbility::HealEffectClass is Invalid"));
	}
	
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealEffectClass, GetAbilityLevel(), EffectContext);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
