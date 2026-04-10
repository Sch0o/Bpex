// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


struct FPositionSnapshot;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	//回溯接口
	//在历史记录中插值获取指定时间的快照
	bool GetSnapshotAtTime(float TargetTime, FPositionSnapshot& OutSnapshot) const;
	//将Actor移动到历史位置
	void RewindTo(float TargetTime);
	//恢复到当前真实位置
	void Restore();
	
	bool IsRewound() const {return bIsRewound;}
	
private:
	void SaveSnapshot();
	//历史位置，按时间升序
	TArray<FPositionSnapshot> History;
	//最大记录时长
	UPROPERTY(EditDefaultsOnly,Category="LagCompensation")
	float MaxRecordTime = 1.0f;
	//记录频率
	UPROPERTY(EditDefaultsOnly,Category="LagCompensation")
	float MinSnapshotInterval = 0.01f;
	
	float LastSnapshotTime = -1.f;
	
	//回溯状态恢复
	bool bIsRewound = false;
	FVector SavedLocation;
	FRotator SavedRotation;
};
