// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/AutoHealAbility.h"
#include "AbilitySystemComponent.h"

UAutoHealAbility::UAutoHealAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAutoHealAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//检查技能是否可以执行
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

	// 创建 GE 上下文和句柄 (Spec)
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealEffectClass, GetAbilityLevel(), EffectContext);

	if (SpecHandle.IsValid())
	{
		// 将 GE 应用到自己身上
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
