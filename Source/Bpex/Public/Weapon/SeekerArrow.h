// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BpexProjectile.h"
#include "SeekerArrow.generated.h"

class UGameplayEffect;

UCLASS()
class BPEX_API ASeekerArrow : public ABpexProjectile
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASeekerArrow();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> RevealEffectClass; // 在蓝图里填入你的 GE_Revealed

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float ScanRadius = 1000.f; // 扫描半径

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float ScanDelay = 2.0f; // 插墙后延迟几秒扫描
	
	UFUNCTION()
	void OnProjectileStop(const FHitResult&ImpactResult);
	
	void ExecuteScan();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
private:
	FTimerHandle ScanTimerHandle;
};
