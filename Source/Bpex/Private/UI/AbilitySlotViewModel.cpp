// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitySlotViewModel.h"

void UAbilitySlotViewModel::SetCooldownTag(FGameplayTag Tag)
{
	CooldownTag = Tag;
}

void UAbilitySlotViewModel::StartCooldown(float InTimeRemaining ,float Duration)
{
	
	TotalDuration = Duration;
	TimeRemaining = InTimeRemaining;
	
	UE_MVVM_SET_PROPERTY_VALUE(CooldownFraction,0.f);
	UE_MVVM_SET_PROPERTY_VALUE(CooldownVisibility, ESlateVisibility::HitTestInvisible);
	UE_MVVM_SET_PROPERTY_VALUE(IconTint, FLinearColor(0.3f, 0.3f, 0.3f, 1.f));
	UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::AsNumber(FMath::CeilToInt(Duration)));

	StopTimer();

	if (CachedWorld.IsValid())
	{
		CachedWorld->GetTimerManager().SetTimer(
			TimerHandle,
			this,
			&UAbilitySlotViewModel::TickCooldown,
			TickInterval,
			true
		);
	}
}

void UAbilitySlotViewModel::EndCooldown()
{
	StopTimer();
	
	UE_MVVM_SET_PROPERTY_VALUE(CooldownFraction, 0.f);
	UE_MVVM_SET_PROPERTY_VALUE(CooldownText, FText::GetEmpty());
	UE_MVVM_SET_PROPERTY_VALUE(CooldownVisibility, ESlateVisibility::Collapsed);
	UE_MVVM_SET_PROPERTY_VALUE(IconTint, FLinearColor::White);
}

void UAbilitySlotViewModel::SetWorldContext(UWorld* World)
{
	CachedWorld = World;
}

void UAbilitySlotViewModel::StopTimer()
{
	UWorld* World = CachedWorld.Get();
	if (World && TimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UAbilitySlotViewModel::TickCooldown()
{
	TimeRemaining -= 0.1f;
	if (TimeRemaining <= 0.f)
	{
		EndCooldown();
		return;
	}
	
	float Fraction = TimeRemaining / TotalDuration;
	UE_MVVM_SET_PROPERTY_VALUE(CooldownFraction, 1 - Fraction);

	FText Text;
	if (TimeRemaining > 1.f)
	{
		Text = FText::AsNumber(FMath::CeilToInt(TimeRemaining));
	}
	else
	{
		FNumberFormattingOptions Opts;
		Opts.MaximumFractionalDigits = 1;
		Opts.MinimumFractionalDigits = 1;
		Text = FText::AsNumber(TimeRemaining, &Opts);
	}
	UE_MVVM_SET_PROPERTY_VALUE(CooldownText, Text);
}
