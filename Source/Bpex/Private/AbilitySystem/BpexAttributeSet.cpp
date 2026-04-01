// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/BpexAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"

void UBpexAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, ClipAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, MaxClipAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, ReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBpexAttributeSet, MaxReserveAmmo, COND_None, REPNOTIFY_Always);

}

void UBpexAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 更简洁的写法，确保不小于1
	}
	else if (Attribute == GetClipAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxClipAmmo());
	}
	else if (Attribute == GetMaxClipAmmoAttribute() || Attribute == GetMaxReserveAmmoAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetReserveAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxReserveAmmo());
	}
}

void UBpexAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		
		if (GetHealth() <= 0.0f)
		{
			AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
			OnOutOfHealth.Broadcast(Instigator);
		}
	}else if (Data.EvaluatedData.Attribute == GetClipAmmoAttribute())
	{
		SetClipAmmo(FMath::Clamp(GetClipAmmo(), 0.0f, GetMaxClipAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetReserveAmmoAttribute())
	{
		SetReserveAmmo(FMath::Clamp(GetReserveAmmo(), 0.0f, GetMaxReserveAmmo()));
	}
}

void UBpexAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data,
                                            FEffectProperties& Props)
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->
		AvatarActor.
		IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();

		if (!IsValid(Props.SourceController) && IsValid(Props.SourceAvatarActor))
		{
			if (APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		
		if (Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void UBpexAttributeSet::OnRep_Health(const FGameplayAttributeData& oldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet, Health, oldHealth);
}

void UBpexAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& oldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet, MaxHealth, oldMaxHealth);
}

void UBpexAttributeSet::OnRep_MaxReserveAmmo(const FGameplayAttributeData& OldMaxReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet, MaxReserveAmmo, OldMaxReserveAmmo);
}

void UBpexAttributeSet::OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet, ReserveAmmo, OldReserveAmmo);
}

void UBpexAttributeSet::OnRep_MaxClipAmmo(const FGameplayAttributeData& OldMaxClipAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet,MaxClipAmmo, OldMaxClipAmmo);
}

void UBpexAttributeSet::OnRep_ClipAmmo(const FGameplayAttributeData& OldClipAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBpexAttributeSet,ClipAmmo, OldClipAmmo);
}
