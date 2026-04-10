// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/LagCompensation/LagCompensationComponent.h"

#include "Weapon/LagCompensation/LagCompensationSubsystem.h"
#include "Weapon/LagCompensation/LagCompensationTypes.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	SetIsReplicatedByDefault(false);
}


// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner()->HasAuthority())
	{
		if (ULagCompensationSubsystem*Sub = GetWorld()->GetSubsystem<ULagCompensationSubsystem>())
		{
			Sub->RegisterComponent(this);
		}
		SaveSnapshot();
	}else
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

void ULagCompensationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner()->HasAuthority())
	{
		if (ULagCompensationSubsystem* Sub =
				GetWorld()->GetSubsystem<ULagCompensationSubsystem>())
		{
			Sub->UnregisterComponent(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveSnapshot();
}

bool ULagCompensationComponent::GetSnapshotAtTime(float TargetTime, FPositionSnapshot& OutSnapshot) const
{
	if (History.Num() == 0) return false;
	if (TargetTime <= History[0].ServerTime)
	{
		OutSnapshot = History[0];
		return true;
	}
	if (TargetTime >= History.Last().ServerTime)
	{
		OutSnapshot = History.Last();
		return true;
	}
	int32 left = 0, right = History.Num() - 1;
	while (right - left > 1)
	{
		int32 Mid = left + (right - left) / 2;
		if (TargetTime <= History[Mid].ServerTime)
		{
			right = Mid;
		}
		else
		{
			left = Mid;
		}
	}
	const float TimeDiff = History[right].ServerTime - History[left].ServerTime;
	const float Alpha = TimeDiff > KINDA_SMALL_NUMBER ? (TargetTime - History[left].ServerTime) / TimeDiff : 0.f;
	OutSnapshot.ServerTime = TargetTime;
	OutSnapshot.Location = FMath::Lerp(History[left].Location, History[right].Location, Alpha);
	OutSnapshot.Rotation = FQuat::Slerp(FQuat(History[left].Rotation), FQuat(History[right].Rotation), Alpha).Rotator();
	return true;
}

void ULagCompensationComponent::RewindTo(float TargetTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || bIsRewound) return;
	FPositionSnapshot Snap;
	if (!GetSnapshotAtTime(TargetTime, Snap))return;
	//保存当前位置
	bIsRewound = true;
	SavedLocation = Owner->GetActorLocation();
	SavedRotation = Owner->GetActorRotation();
	Owner->SetActorLocationAndRotation(Snap.Location,Snap.Rotation,false, nullptr, ETeleportType::TeleportPhysics);
}

void ULagCompensationComponent::Restore()
{
	if (!bIsRewound) return;
	AActor* Owner = GetOwner();
	if (!Owner) return;
	Owner->SetActorLocationAndRotation(SavedLocation,SavedRotation,false, nullptr, ETeleportType::TeleportPhysics);
	bIsRewound = false;
}

void ULagCompensationComponent::SaveSnapshot()
{
	UWorld* World = GetWorld();
	if (!World) return;
	const float NowTime = World->GetTimeSeconds();
	//频率限制
	if (NowTime - LastSnapshotTime < MinSnapshotInterval)return;
	LastSnapshotTime = NowTime;
	//清除过老的快照
	const float CutoffTime = NowTime - MaxRecordTime;
	int32 NumToRemove = 0;
	for (int32 i = 0; i < History.Num(); i++)
	{
		if (History[i].ServerTime < CutoffTime)
		{
			NumToRemove++;
		}
		else
		{
			break;
		}
	}
	if (NumToRemove > 0)
	{
		History.RemoveAt(0, NumToRemove, EAllowShrinking::No);
	}
	//记录新快照
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FPositionSnapshot Snap;
	Snap.ServerTime = NowTime;
	Snap.Location = Owner->GetActorLocation();
	Snap.Rotation = Owner->GetActorRotation();
	History.Add(Snap);
}
