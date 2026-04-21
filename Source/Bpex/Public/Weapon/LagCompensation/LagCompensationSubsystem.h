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
	
	// 回溯指定Actor的HitBox
	bool RewindActor(AActor* Actor, float RewindTime);
	// 恢复指定Actor的HitBox
	void RestoreActor(AActor* Actor);
	
	// 回溯所有已注册的目标（除了指定的射击者）
	void RewindAllExcept(AActor* ExceptActor, float RewindTime);
	// 恢复所有已回溯的目标
	void RestoreAll();
	
	ULagCompensationComponent* FindComponentForActor(AActor* Actor) const;

	int32 GetRegisteredCount() const { return RegisteredComponents.Num(); }
	
	void DrawCurrentPosition();
	
private:
	TArray<TWeakObjectPtr<ULagCompensationComponent>> RegisteredComponents;
	//清除已失效的弱引用
	void PurgeStaleEntries();
};
