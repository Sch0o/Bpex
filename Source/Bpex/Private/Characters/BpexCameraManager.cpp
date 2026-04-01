// Copyright Epic Games, Inc. All Rights Reserved.


#include "Characters/BpexCameraManager.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABpexCameraManager::ABpexCameraManager()
{
	// set the min/max pitch
	ViewPitchMin = -70.0f;
	ViewPitchMax = 80.0f;
}

void ABpexCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	ACharacter* Character = Cast<ACharacter>(GetOwningPlayerController()->GetPawn());
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	//从类默认对象得到初始高度
	FVector TargetCrouchOffset = FVector(
		0, 0, MovementComponent->GetCrouchedHalfHeight() - Character->GetClass()->GetDefaultObject<ACharacter>()->
		                                                              GetCapsuleComponent()->
		                                                              GetScaledCapsuleHalfHeight());

	FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset,
	                             FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

	if (MovementComponent->IsCrouching())
	{
		CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
		Offset -= TargetCrouchOffset;
	}
	else
	{
		CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
	}

	if (MovementComponent->IsMovingOnGround())
	{
		OutVT.POV.Location += Offset;
	}
}
