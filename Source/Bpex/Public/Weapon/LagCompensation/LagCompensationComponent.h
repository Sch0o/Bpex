// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationTypes.h"
#include "LagCompensationComponent.generated.h"


class UBulletDataAsset;
struct FBulletHitReport;
class UBoxComponent;
struct FPositionSnapshot;
struct FFrameSnapshot;

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
	//在历史记录中插值获取指定时间的快照
	bool GetSnapshotAtTime(const float Time, FFrameSnapshot& OutSnapshot) const;

	FFrameSnapshot InterpolateSnapshots(const FFrameSnapshot& A, const FFrameSnapshot& B, float Alpha) const;

	// 将所有HitBox回溯到历史位置
	void RewindTo(float TargetTime);

	//恢复到当前真实位置
	void Restore();

	bool IsRewound() const { return bIsRewound; }

private:
	UPROPERTY()
	TArray<UBoxComponent*> TrackedBoxes;

	//找到角色所有的碰撞盒
	void DiscoverHitBoxes();
	//保存新快照
	void SaveSnapshot();
	//清除过期的快照
	void PruneHistory(float CurrentTime);

	TArray<FFrameSnapshot> History;

	//最大记录时长
	UPROPERTY(EditDefaultsOnly, Category="LagCompensation")
	float MaxRecordTime = 1.0f;
	//记录频率
	UPROPERTY(EditDefaultsOnly, Category="LagCompensation")
	float MinSnapshotInterval = 0.01f;

	float LastSnapshotTime = -1.f;

	//回溯状态恢复
	bool bIsRewound = false;

	UPROPERTY()
	FFrameSnapshot SavedCurrentSnapshot;
};
