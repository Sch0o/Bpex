// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ShooterEnemyDummy.generated.h"

struct FOnAttributeChangeData;
class UGameplayEffect;
class UGameplayAbility;

UCLASS()
class BPEX_API AShooterEnemyDummy : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	AShooterEnemyDummy();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "GAS")
	class UBpexAttributeSet*AttributeSet;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;
	
	void InitializePrimaryAttributes() const;
	
	virtual void HealthChanged(const FOnAttributeChangeData&Data);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
