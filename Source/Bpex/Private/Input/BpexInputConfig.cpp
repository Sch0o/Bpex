// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/BpexInputConfig.h"

const UInputAction* UBpexInputConfig::FindAbilityInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FBpexInputAction& Action : AbilityInputActions)
	{
		if (Action.InputTag == InputTag && Action.InputAction)
		{
			return Action.InputAction;
		}
	}
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s], on InputConfig [%s]"),
		       *InputTag.ToString(), *GetNameSafe(this));
	}
	return nullptr;
}
