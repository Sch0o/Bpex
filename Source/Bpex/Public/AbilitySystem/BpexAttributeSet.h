// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BpexAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOutOfHealthDelegate, AActor*, InstigatorActor);

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties()
	{
	}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> SourceController;

	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetASC;

	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> TargetController;

	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter;
};

UCLASS()
class BPEX_API UBpexAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnOutOfHealthDelegate OnOutOfHealth;

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ClipAmmo, Category="Weapon Attributes")
	FGameplayAttributeData ClipAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, ClipAmmo)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxClipAmmo, Category="Weapons Attributes")
	FGameplayAttributeData MaxClipAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, MaxClipAmmo)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReserveAmmo, Category="Weapons Attributes")
	FGameplayAttributeData ReserveAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, ReserveAmmo)
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxReserveAmmo, Category="Weapons Attributes")
	FGameplayAttributeData MaxReserveAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, MaxReserveAmmo)

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UFUNCTION()
	void OnRep_MaxReserveAmmo(const FGameplayAttributeData& OldMaxReserveAmmo);
	
	UFUNCTION()
	void OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo);
	
	UFUNCTION()
	void OnRep_MaxClipAmmo(const FGameplayAttributeData& OldMaxClipAmmo);
	
	UFUNCTION()
	void OnRep_ClipAmmo(const FGameplayAttributeData& OldClipAmmo);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties);
};
