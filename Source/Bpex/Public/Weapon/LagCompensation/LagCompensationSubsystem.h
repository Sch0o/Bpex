// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LagCompensationSubsystem.generated.h"


class ULagCompensationComponent;

UCLASS()
class BPEX_API ULagCompensationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//注册
	void RegisterComponent(ULagCompensationComponent* Component);
	void UnregisterComponent(ULagCompensationComponent* Component);
	//批量回溯
	void RewindAllTargets(float ServerTime, AActor* Instigator = nullptr);
	void RestoreAllTargets();
	bool RewindLineTrace(FHitResult& OutHit, const FVector& Start, const FVector& End, ECollisionChannel Channel,
	                     const FCollisionQueryParams& Params, float RewindToTime, AActor* Instigator = nullptr);
	bool RewindSweep(FHitResult& OutHit, const FVector& Start, const FVector& End, const FCollisionShape& Shape,
	                 ECollisionChannel Channel, const FCollisionQueryParams& Params, float RewindToTime,
	                 AActor* Instigator = nullptr);

	int32 GetRegisteredCount() const { return RegisteredComponents.Num(); }
	
	void DrawCurrentPosition();
private:
	TArray<TWeakObjectPtr<ULagCompensationComponent>> RegisteredComponents;
	//清除已失效的弱引用
	void PurgeStaleEntries();
};
