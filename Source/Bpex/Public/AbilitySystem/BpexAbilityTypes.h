#pragma once

#include"CoreMinimal.h"
#include "BpexAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EBpexAbilityActivationPolicy:uint8
{
	OnInputTriggered UMETA(DisplayName = "On Input Triggered"),

	// 按住持续触发 (适合全自动步枪、持续施法)
	WhileInputActive UMETA(DisplayName = "While Input Active"),

	// 赋予技能时自动激活 (适合被动技能、光环效果)
	OnSpawn UMETA(DisplayName = "On Spawn")
};
