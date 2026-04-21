// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletTypes.h"
#include "GameplayEffectTypes.h"
#include "Components/ActorComponent.h"
#include "BulletManagerComponent.generated.h"

class ULagCompensationSubsystem;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBulletHit, const FHitResult&, HitResult, const FActiveBullet&, Bullet);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BPEX_API UBulletManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBulletManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Bullet")
	int32 FireBullet(const UBulletDataAsset* Config, const FVector& Origin, const FVector& Direction,
	                  const FGameplayEffectSpecHandle& DamageSpec);

	UPROPERTY(BlueprintAssignable, Category="Bullet")
	FOnBulletHit OnBulletHit;

	UFUNCTION(BlueprintPure, Category="Bullet")
	int32 GetActiveBulletCount() const { return ActiveBullets.Num(); }

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_NotifyFire(FFireParams Params, const UBulletDataAsset* Config);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ReportHit(FBulletHitReport Report, const UBulletDataAsset* Config);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnTracerVFX(FVector Origin, FVector Direction, const UBulletDataAsset* Config);
	
	UFUNCTION(Client, Unreliable) 
	void Client_DrawServerTrajectory(FVector ServerStart, FVector ServerEnd);

private:
	TArray<FActiveBullet> ActiveBullets;
	uint32 NextBulletID = 0;

	void SimulateBullet(FActiveBullet& Bullet, float DeltaTime);
	void ProcessLocalHit(FActiveBullet& Bullet, const FHitResult& HitResult);
	void CleanupDeadBullets();
	void SpawnTracer(FActiveBullet& Bullet);
	void UpdateTracer(FActiveBullet& Bullet);
	bool ServerValidateHit(const FBulletHitReport& Report,
					   const UBulletDataAsset* Config,
					   FHitResult& OutValidatedHit);
	void ServerApplyDamage(const FBulletHitReport&Report,const UBulletDataAsset* Config,FHitResult ValidatedHit);
	
	//执行子弹碰撞检测
	bool PerformBulletTrace(FActiveBullet& Bullet, const FVector& Start, const FVector& End,FHitResult& OutHit,TEnumAsByte<ECollisionChannel> CollisionChannel);
	
	//更新子弹参数
	bool UpdateBulletParams(FActiveBullet& BulletS,float DeltaTime);
	
	float GetOwnerHalfRTT() const;
	
	//预测弹道和碰撞
	bool PredictCustomProjectilePath(   const FActiveBullet& InitialBulletState, 
	float MaxSimulateTime, 
	float SimFrequency, 
	TArray<FVector>& OutPathPositions, 
	FHitResult& OutHit);
	
	UPROPERTY()
	TObjectPtr<ULagCompensationSubsystem> LagCompSubsystem = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	float MaxCatchUpTime = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	float ExtraRewindTime = 0.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	bool bEnableTargetRewind = true;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TEnumAsByte<ECollisionChannel> BulletTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	int32 MaxActiveBullets = 128;
	float GetWorldGravity() const;
	
	
	// 命中容差（服务器模拟命中点与客户端报告命中点的最大允许偏差）
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|Validation")
	float HitValidationTolerance = 150.f;
	// 起始位置容差（防止客户端伪造发射点）
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|Validation")
	float MaxOriginDeviation = 300.f;
};
