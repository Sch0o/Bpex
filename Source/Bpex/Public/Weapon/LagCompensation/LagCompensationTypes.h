#pragma once
#include "CoreMinimal.h"
#include "LagCompensationTypes.generated.h"

USTRUCT()
struct BPEX_API FPositionSnapshot
{
	GENERATED_BODY()
	
	UPROPERTY()
	float ServerTime = 0.f;
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
};

