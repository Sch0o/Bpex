// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/LagCompensation/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
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
	
	DiscoverHitBoxes();
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

void ULagCompensationComponent::RewindTo(float TargetTime)
{
	if (bIsRewound) return;
	FFrameSnapshot HistorySnap;
	if (!GetSnapshotAtTime(TargetTime, HistorySnap)) return;
	// 保存当前真实位置
	SavedCurrentSnapshot.Timestamp = GetWorld()->GetTimeSeconds();
	SavedCurrentSnapshot.HitBoxes.Empty();
	for (UBoxComponent* Box : TrackedBoxes)
	{
		if (!Box) continue;
		const FName BoxName = Box->GetFName();
		// 保存当前位置
		FHitBoxSnapshot CurrentSnap;
		CurrentSnap.Location = Box->GetComponentLocation();
		CurrentSnap.Rotation = Box->GetComponentQuat();
		CurrentSnap.Extent= Box->GetScaledBoxExtent();
		CurrentSnap.BoneName = Box->GetAttachSocketName();
		SavedCurrentSnapshot.HitBoxes.Add(BoxName, CurrentSnap);
		// 移动到历史位置
		const FHitBoxSnapshot* HistoryBox = HistorySnap.HitBoxes.Find(BoxName);
		if (HistoryBox)
		{
			Box->SetWorldLocationAndRotation(
				HistoryBox->Location,
				HistoryBox->Rotation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}
	}
	bIsRewound = true;
	UE_LOG(LogTemp, Verbose,
		   TEXT("LagComp: Rewound %s to time %.3f (%d boxes)"),
		   *GetOwner()->GetName(), TargetTime, TrackedBoxes.Num());
}

void ULagCompensationComponent::Restore()
{
	if (!bIsRewound) return;
	for (UBoxComponent* Box : TrackedBoxes)
	{
		if (!Box) continue;
		const FName BoxName = Box->GetFName();
		const FHitBoxSnapshot* Saved = SavedCurrentSnapshot.HitBoxes.Find(BoxName);
		if (Saved)
		{
			Box->SetWorldLocationAndRotation(
				Saved->Location,
				Saved->Rotation,
				false,
				nullptr,
				ETeleportType::TeleportPhysics);
		}
	}
	bIsRewound = false;
	UE_LOG(LogTemp, Verbose,
		   TEXT("LagComp: Restored %s (%d boxes)"),
		   *GetOwner()->GetName(), TrackedBoxes.Num());
}

void ULagCompensationComponent::DiscoverHitBoxes()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	// 获取角色身上所有的 BoxComponent
	TArray<UBoxComponent*> AllBoxes;
	Owner->GetComponents<UBoxComponent>(AllBoxes);
	for (UBoxComponent* Box : AllBoxes)
	{
		FName BoxName = Box->GetFName();
		if (BoxName.ToString().StartsWith("HitBox_"))
		{
			TrackedBoxes.Add(Box);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("LagComp: Found %d hit boxes on %s"),TrackedBoxes.Num(), *Owner->GetName());
}

void ULagCompensationComponent::SaveSnapshot()
{
	UWorld* World = GetWorld(); if (!World) return;
	const float CurrentTime = World->GetTimeSeconds();
	
	//频率限制
	if (CurrentTime - LastSnapshotTime < MinSnapshotInterval)return;
	LastSnapshotTime = CurrentTime;
	
	//记录新快照
	FFrameSnapshot Snapshot;
	Snapshot.Timestamp = CurrentTime;
	Snapshot.HitBoxes.Reserve(TrackedBoxes.Num());
	for (int32 i = 0; i < TrackedBoxes.Num(); ++i)
	{
		UBoxComponent* Box = TrackedBoxes[i];
		if (!Box) continue;
		FHitBoxSnapshot BoxSnap;
		BoxSnap.Location= Box->GetComponentLocation();
		BoxSnap.Rotation        = Box->GetComponentQuat();
		BoxSnap.Extent          = Box->GetScaledBoxExtent();
		BoxSnap.BoneName        = Box->GetAttachSocketName();
		FName BoxName = Box->GetFName();
		Snapshot.HitBoxes.Add(BoxName, BoxSnap);
	}
	History.Add(MoveTemp(Snapshot));
	
	//清除过老的快照
	PruneHistory(CurrentTime);

}

void ULagCompensationComponent::PruneHistory(float CurrentTime)
{
	const float CutoffTime = CurrentTime - MaxRecordTime;
	int32 NumToRemove = 0;
	for (int32 i = 0; i < History.Num(); i++)
	{
		if (History[i].Timestamp < CutoffTime)
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
}


bool ULagCompensationComponent::GetSnapshotAtTime(const float Time, FFrameSnapshot& OutSnapshot) const
{
	if (History.Num() == 0) return false;
	if (Time <= History[0].Timestamp)      { OutSnapshot = History[0];      return true; }
	if (Time >= History.Last().Timestamp)   { OutSnapshot = History.Last();  return true; }
	int32 Lo = 0, Hi = History.Num() - 1;
	while (Hi - Lo > 1)
	{
		int32 Mid = (Lo + Hi) / 2;
		if (History[Mid].Timestamp <= Time) Lo = Mid;
		else Hi = Mid;
	}
	float Span = History[Hi].Timestamp - History[Lo].Timestamp;
	float Alpha = (Span > KINDA_SMALL_NUMBER)  ? (Time - History[Lo].Timestamp) / Span
				  : 0.f;
	OutSnapshot = InterpolateSnapshots(History[Lo], History[Hi],FMath::Clamp(Alpha, 0.f, 1.f));
	OutSnapshot.Timestamp = Time;
	return true;
}

FFrameSnapshot ULagCompensationComponent::InterpolateSnapshots(
	const FFrameSnapshot& A, const FFrameSnapshot& B, float Alpha) const
{
	FFrameSnapshot Result;
	for (const auto& Pair : A.HitBoxes)
	{
		FName Key = Pair.Key;
		const FHitBoxSnapshot& BA = Pair.Value;
		// 在B中查找同名碰撞盒
		const FHitBoxSnapshot* BB = B.HitBoxes.Find(Key);
		FHitBoxSnapshot Out;
		if (BB)
		{
			Out.Location = FMath::Lerp(BA.Location, BB->Location, Alpha);
			Out.Rotation = FQuat::Slerp(BA.Rotation, BB->Rotation, Alpha);
			Out.Extent   = BA.Extent;
			Out.BoneName = BA.BoneName;
		}
		else
		{
			Out = BA;
		}
		Result.HitBoxes.Add(Key, Out);
	}
	return Result;
}
