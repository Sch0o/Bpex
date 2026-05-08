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
		UE_LOG(LogTemp, Error, TEXT("ULegendAbilityComponent::InitAbilitySystem:: InASC cannot be null"));
		return;
	}
	ASC = InASC;


	InitLegendData();
}

void ULegendAbilityComponent::InitLegendData()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return; // 仅限服务器执行
	}

	if (!ASC || !LegendData)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitLegendData failed: ASC or LegendData is null."));
		return;
	}

	GrantedAbilityHandles.Empty();

	//赋予角色技能
	if (LegendData->TacticalAbility.AbilityClass)
	{
		FGameplayAbilitySpec Spec(LegendData->TacticalAbility.AbilityClass, 1, INDEX_NONE, GetOwner());

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);

		GrantedAbilityHandles.Add(LegendData->TacticalAbility.SlotType, Handle);
	}

	if (LegendData->UltimateAbility.AbilityClass)
	{
		FGameplayAbilitySpec Spec(LegendData->UltimateAbility.AbilityClass, 1, -1, GetOwner());

		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		GrantedAbilityHandles.Add(LegendData->UltimateAbility.SlotType, Handle);
	}
}
