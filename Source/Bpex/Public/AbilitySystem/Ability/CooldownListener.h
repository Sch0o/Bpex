// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "CooldownListener.generated.h"


struct FGameplayEffectSpec;
DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnCooldownChangedNative,
	FGameplayTag /*CooldownTag*/,
	float /*TimeRemaining*/,
	float /*Duration*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCooldownChangedDynamic,
											   FGameplayTag, CooldownTag,
											   float, TimeRemaining,
											   float, Duration);

UCLASS()
class BPEX_API UCooldownListener : public UObject
{
	GENERATED_BODY()

public:
	// 原生委托（C++绑定用，不需要UFUNCTION）
	FOnCooldownChangedNative OnCooldownBeginNative;
	FOnCooldownChangedNative OnCooldownEndNative;
	
	UPROPERTY(BlueprintAssignable)
	FOnCooldownChangedDynamic OnCooldownBeginDynamic;
	UPROPERTY(BlueprintAssignable)
	FOnCooldownChangedDynamic OnCooldownEndDynamic;

	void StartListening(
		UAbilitySystemComponent* InASC,
		const FGameplayTagContainer& InCooldownTags,
		bool bInUseServerCooldown);

	void StopListening();
	
	bool IsListening() const { return bIsListening; }

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	
	FGameplayTagContainer CooldownTags;
	
	bool bUseServerCooldown = false;
	
	bool bIsListening = false;
	// GE 添加回调
	void OnActiveGameplayEffectAdded(
		UAbilitySystemComponent* Target,
		const FGameplayEffectSpec& SpecApplied,
		FActiveGameplayEffectHandle ActiveHandle);
	// Tag 变化回调
	void OnCooldownTagChanged(
		const FGameplayTag CooldownTag,
		int32 NewCount);
	// 获取CD剩余时间
	bool GetCooldownRemainingForTag(
		const FGameplayTagContainer& InTags,
		float& OutTimeRemaining,
		float& OutDuration) const;
	// 广播辅助函数
	void BroadcastCooldownBegin(
		FGameplayTag Tag, float TimeRemaining, float Duration);
	
	void BroadcastCooldownEnd(FGameplayTag Tag);
};
