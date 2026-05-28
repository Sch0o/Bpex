#pragma once
#include "CoreMinimal.h"
#include "BpexTypes.generated.h"

UENUM(BlueprintType)
enum class EGate : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Jogging UMETA(DisplayName = "Jogging"),
	Crouching UMETA(DisplayName = "Crouching")
};

UENUM(BlueprintType)
enum class EGun : uint8
{
	UnArmed UMETA(DisplayName = "UnArmed"),
	Pistol UMETA(DisplayName = "Pistol"),
	Rifle UMETA(DisplayName = "Rifle")
};

USTRUCT(BlueprintType)
struct FGateSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float MaxWalkSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float MaxAcceleration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float BrakingDeceleration = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float BrakingFrictionFactor = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	float BrakingFriction = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Data")
	bool UseSeparateBrakingFriction = true;
};






