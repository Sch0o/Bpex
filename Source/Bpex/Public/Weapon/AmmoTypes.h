#pragma once
#include "CoreMinimal.h"
#include "AmmoTypes.generated.h"

UENUM(BlueprintType)
enum class EAmmoType:uint8
{
	None     UMETA(DisplayName = "None"),
	Light    UMETA(DisplayName = "Light"),
	Heavy    UMETA(DisplayName = "Heavy"),
}; 