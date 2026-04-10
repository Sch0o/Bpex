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
	void Server_FireBullet(FFireParams Params, const UBulletDataAsset* Config);
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnTracerVFX(FVector Origin, FVector Direction, const UBulletDataAsset* Config);
	
	UFUNCTION(Client, Unreliable) 
	void Client_DrawServerTrajectory(FVector ServerStart, FVector ServerEnd);

private:
	TArray<FActiveBullet> ActiveBullets;
	uint32 NextBulletID = 0;

	void SimulateBullet(FActiveBullet& Bullet, float DeltaTime);
	void ProcessHit(FActiveBullet& Bullet, const FHitResult& HitResult);
	void CleanupDeadBullets();
	void SpawnTracer(FActiveBullet& Bullet);
	void UpdateTracer(FActiveBullet& Bullet);
	
	//延迟补偿
	void FastForwardBullet(FActiveBullet& Bullet, float DeltaTime);
	bool PerformBulletTrace(FActiveBullet& Bullet, const FVector& Start, const FVector& End,FHitResult& OutHit,float RewindtoTime=-1.f);
	float GetOwnerHalfRTT() const;
	
	UPROPERTY()
	TObjectPtr<ULagCompensationSubsystem> LagCompSubsystem = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	float MaxCatchUpTime = 0.3f;
	/** 额外回溯时间（补偿客户端插值延迟，通常 0~0.1s） */
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	float ExtraRewindTime = 0.0f;
	/** 是否启用目标回溯（关闭则只做子弹快进） */
	UPROPERTY(EditDefaultsOnly, Category = "Bullet|LagCompensation")
	bool bEnableTargetRewind = true;

	// ── 碰撞通道 ──
	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	TEnumAsByte<ECollisionChannel> BulletTraceChannel = ECC_Visibility;
	// ── 性能限制 ──
	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	int32 MaxActiveBullets = 128;
	/** 获取重力值 */
	float GetWorldGravity() const;
};
