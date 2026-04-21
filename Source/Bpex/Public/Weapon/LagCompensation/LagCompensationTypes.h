#pragma once
#include "CoreMinimal.h"
#include "LagCompensationTypes.generated.h"

USTRUCT()
struct FHitBoxSnapshot
{
	GENERATED_BODY()
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	FQuat Rotation = FQuat::Identity;
	UPROPERTY()
	FVector Extent = FVector::ZeroVector;
	UPROPERTY()
	FName BoneName;
};


USTRUCT()
struct FFrameSnapshot
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Timestamp = 0.f;
	
	UPROPERTY()
	TMap<FName, FHitBoxSnapshot> HitBoxes;
};

