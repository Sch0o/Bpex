// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "BpexCameraManager.generated.h"

/**
 *  Basic First Person camera manager.
 *  Limits min/max look pitch.
 */
UCLASS()
class ABpexCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration=0.5f;
	
	float CrouchBlendTime;
	
public:
	
	ABpexCameraManager();
	
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
