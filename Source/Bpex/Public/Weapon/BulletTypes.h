// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "GameplayEffectTypes.h"
#include "BulletTypes.generated.h"

struct FGameplayEffectSpecHandle;
class UGameplayEffect;

UCLASS()
class BPEX_API UBulletDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Ballistics ")
	float Speed = 29000.f;

	UPROPERTY(EditDefaultsOnly, Category="Ballistics ")
	float GravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ballistics ")
	float CollisionRadius = 2.f;

	UPROPERTY(EditDefaultsOnly, Category="Ballistics ")
	float MaxLifetime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="Ballistics ")
	float MaxDistance = 50000.f;

	//Damage
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BaseDamage = 14.f; // R-301单发伤害

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	TSubclassOf<UGameplayEffect> AmmoCostEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UNiagaraSystem* TracerSystem = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UNiagaraSystem* ImpactEffect = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	float TracerSpeed = 35000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadAngle = 1.5f; // 度
};

//运行时子弹实例
USTRUCT()
struct FActiveBullet
{
	GENERATED_BODY()

	FVector Position = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	FVector StartPosition = FVector::ZeroVector;

	UPROPERTY()
	const UBulletDataAsset* Config = nullptr;

	float LifeRemaining = 3.f;
	float DistanceTraveled = 0.f;
	bool bPendingKill = false;

	FGameplayEffectSpecHandle DamageSpecHandle;

	UPROPERTY()
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> InstigatorASC;

	uint32 BulletID = 0;

	bool bIsLocalPrediction = false;
	bool bIsAuthority = false;

	UPROPERTY()
	TWeakObjectPtr<UNiagaraComponent> TracerComponent;

	float OwnerHalfRTT = 0.f;
	float ServerTime = 0.f;
	float FireServerTime = 0.f;
	
	FVector StartVelocity = FVector::ZeroVector;
};

USTRUCT()
struct FFireParams
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Origin = FVector::ZeroVector;

	UPROPERTY()
	FVector Direction = FVector::ZeroVector;

	UPROPERTY()
	float ServerTime = 0.f;

	UPROPERTY()
	uint32 BulletID = 0;
};


USTRUCT()
struct FBulletHitReport
{
	GENERATED_BODY()
	UPROPERTY()
	uint32 BulletID = 0;
	UPROPERTY()
	TWeakObjectPtr<AActor> HitActor;
	UPROPERTY();
	FVector_NetQuantize StartLocation;
	UPROPERTY()
	FVector_NetQuantize StartVelocity;
	UPROPERTY()
	float FireServerTime = 0.f;
	UPROPERTY()
	FVector_NetQuantize ClientHitLocation;
};
