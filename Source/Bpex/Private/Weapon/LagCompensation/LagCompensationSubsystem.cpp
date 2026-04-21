// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/LagCompensation/LagCompensationSubsystem.h"
#include "Weapon/LagCompensation/LagCompensationComponent.h"

void ULagCompensationSubsystem::RegisterComponent(ULagCompensationComponent* Component)
{
	if (Component)
	{
		RegisteredComponents.AddUnique(Component);
		UE_LOG(LogTemp, Log, TEXT("LagComp: Registered %s (%d total)"),
		       *GetNameSafe(Component->GetOwner()), RegisteredComponents.Num());
	}
}

void ULagCompensationSubsystem::UnregisterComponent(ULagCompensationComponent* Component)
{
	RegisteredComponents.Remove(Component);
}

bool ULagCompensationSubsystem::RewindActor(AActor* Actor, float RewindTime)
{
	ULagCompensationComponent* Comp = FindComponentForActor(Actor);
	if (!Comp) return false;
	UWorld* World = GetWorld();
	if (!World) return false;
	const float CurrentServerTime = World->GetTimeSeconds();
	const float TargetTime = CurrentServerTime - RewindTime;
	Comp->RewindTo(TargetTime);
	return Comp->IsRewound();
}

void ULagCompensationSubsystem::RestoreActor(AActor* Actor)
{
	ULagCompensationComponent* Comp = FindComponentForActor(Actor);
	if (Comp && Comp->IsRewound())
	{
		Comp->Restore();
	}
}

void ULagCompensationSubsystem::RewindAllExcept(AActor* ExceptActor, float RewindTime)
{
	PurgeStaleEntries();

	UWorld* World = GetWorld();
	if (!World) return;
	const float CurrentServerTime = World->GetTimeSeconds();
	const float TargetTime = CurrentServerTime - RewindTime;
	for (const auto& Weak : RegisteredComponents)
	{
		ULagCompensationComponent* Comp = Weak.Get();
		if (!Comp || !Comp->GetOwner()) continue;
		if (Comp->GetOwner() == ExceptActor) continue;
		Comp->RewindTo(TargetTime);
	}
}

void ULagCompensationSubsystem::RestoreAll()
{
	for (const auto& Weak : RegisteredComponents)
	{
		ULagCompensationComponent* Comp = Weak.Get();
		if (Comp && Comp->IsRewound())
		{
			Comp->Restore();
		}
	}
}

ULagCompensationComponent* ULagCompensationSubsystem::FindComponentForActor(
	AActor* Actor) const
{
	if (!Actor) return nullptr;
	for (const auto& Weak : RegisteredComponents)
	{
		ULagCompensationComponent* Comp = Weak.Get();
		if (Comp && Comp->GetOwner() == Actor)
		{
			return Comp;
		}
	}
	return nullptr;
}

void ULagCompensationSubsystem::PurgeStaleEntries()
{
	RegisteredComponents.RemoveAll([](const TWeakObjectPtr<ULagCompensationComponent>& Ptr)
	{
		return !Ptr.IsValid();
	});
}
