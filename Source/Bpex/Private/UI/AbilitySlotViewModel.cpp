// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySlotViewModel.h"

void UAbilitySlotViewModel::StartCooldown(UWorld* World, float InTimeRemaining, float InDuration)
{
	
	UE_MVVM_SET_PROPERTY_VALUE(bIsOnCooldown, true);
	
	if (InTimeRemaining <= 0.f)
	{
		UE_MVVM_SET_PROPERTY_VALUE(Fraction, 1.f);
		UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::GetEmpty());
	}else
	{
		UE_MVVM_SET_PROPERTY_VALUE(Duration, InDuration);
		UE_MVVM_SET_PROPERTY_VALUE(TimeRemaining, InTimeRemaining);
		FString TimeStr = FString::Printf(TEXT("%.1fs"), InTimeRemaining);
		UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::FromString(TimeStr));
	}
	
	if (InDuration > 0.f)
	{
		UE_MVVM_SET_PROPERTY_VALUE(Fraction, InTimeRemaining / InDuration);
	}
	
	CachedWorld = World;
	if (!World) return;
	StopTimer();
	World->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UAbilitySlotViewModel::TickCooldown,
		TickInterval,
		true
	);
}

void UAbilitySlotViewModel::StopTimer()
{
	UWorld* World = CachedWorld.Get();
	if (World&&TimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UAbilitySlotViewModel::TickCooldown()
{
	
	float NewTime = FMath::Max(0.f, TimeRemaining - TickInterval);
	UE_MVVM_SET_PROPERTY_VALUE(TimeRemaining, NewTime);
	
	if (Duration > 0.f)
	{
		UE_MVVM_SET_PROPERTY_VALUE(Fraction, NewTime / Duration);
	}
	FString TimeStr = FString::Printf(TEXT("%.1fs"), NewTime);
	UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::FromString(TimeStr));
	
	if (NewTime <= 0.f)
	{
		StopTimer();
		UE_MVVM_SET_PROPERTY_VALUE(TimeRemaining, 0.f);
		UE_MVVM_SET_PROPERTY_VALUE(Duration, 0.f);
		UE_MVVM_SET_PROPERTY_VALUE(Fraction, 0.f);
		UE_MVVM_SET_PROPERTY_VALUE(bIsOnCooldown, false);
		UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::GetEmpty());
	}
}
