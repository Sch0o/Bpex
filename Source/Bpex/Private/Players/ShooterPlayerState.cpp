// Fill out your copyright notice in the Description page of Project Settings.


#include "Players/ShooterPlayerState.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "AbilitySystem/BpexAttributeSet.h"

AShooterPlayerState::AShooterPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBpexAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<UBpexAttributeSet>("AttributeSet");
	
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* AShooterPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSet* AShooterPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}