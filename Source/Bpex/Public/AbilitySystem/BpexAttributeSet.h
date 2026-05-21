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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldBrokenDelegate,AActor*, InstigatorActor);

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
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnShieldBrokenDelegate OnShieldBroken;
	
	

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, MaxHealth)
	
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_LightAmmo, Category="Ammo Attributes")
	FGameplayAttributeData LightAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, LightAmmo)
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_HeavyAmmo, Category="Ammo Attributes")
	FGameplayAttributeData HeavyAmmo;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, HeavyAmmo)
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_Shield, Category="Ammo Attributes")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, Shield)
	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing=OnRep_MaxShield, Category="Ammo Attributes")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, MaxShield)
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData IncomingDamage; 
	ATTRIBUTE_ACCESSORS(UBpexAttributeSet, IncomingDamage)
	
	

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UFUNCTION()
	void OnRep_LightAmmo(const FGameplayAttributeData& OldLightAmmo);
	
	UFUNCTION()
	void OnRep_HeavyAmmo(const FGameplayAttributeData& OldHeavyAmmo);
	
	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield);
	
	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);
	
	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties);
};
