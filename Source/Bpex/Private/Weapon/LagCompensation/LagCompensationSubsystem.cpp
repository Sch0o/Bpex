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

void ULagCompensationSubsystem::PurgeStaleEntries()
{
	RegisteredComponents.RemoveAll([](const TWeakObjectPtr<ULagCompensationComponent>& Ptr)
	{
		return !Ptr.IsValid();
	});
}

void ULagCompensationSubsystem::RewindAllTargets(float ServerTime, AActor* Instigator)
{
	PurgeStaleEntries();
	for (auto&WeakComp:RegisteredComponents)
	{
		if (ULagCompensationComponent*Comp = WeakComp.Get())
		{
			if (Instigator&&Comp->GetOwner()==Instigator)
				continue;
			AActor* Target = Comp->GetOwner();
			FVector BeforePos = Target->GetActorLocation();
			Comp->RewindTo(ServerTime);
			
			FVector AfterPos = Target->GetActorLocation();
			if (FVector::Dist(BeforePos, AfterPos) > 1.f)
			{
				// 绿色=当前位置,蓝色=回溯位置, 白线连接
				DrawDebugSphere(GetWorld(), BeforePos, 30.f, 8,FColor::Green, false, 0.5f);
				DrawDebugSphere(GetWorld(), AfterPos, 30.f, 8,
								FColor::Blue, false, 0.5f);
				DrawDebugLine(GetWorld(), BeforePos, AfterPos,
							  FColor::White, false, 0.5f, 0, 1.f);
			}
		}
	}
}

void ULagCompensationSubsystem::RestoreAllTargets()
{
	for (auto&WeakComp:RegisteredComponents)
	{
		if (ULagCompensationComponent*Comp = WeakComp.Get())
		{
			//如果在回溯状态就恢复
			if (Comp->IsRewound())
			{
				Comp->Restore();
			}
		}
	}
}

bool ULagCompensationSubsystem::RewindLineTrace(FHitResult& OutHit, const FVector& Start, const FVector& End,
	ECollisionChannel Channel, const FCollisionQueryParams& Params, float RewindToTime, AActor* Instigator)
{
	//回溯
	RewindAllTargets(RewindToTime, Instigator);
	//射线检测
	bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, Channel, Params);
	//恢复
	RestoreAllTargets();
	return bHit;
}

bool ULagCompensationSubsystem::RewindSweep(FHitResult& OutHit, const FVector& Start, const FVector& End,
	const FCollisionShape& Shape, ECollisionChannel Channel, const FCollisionQueryParams& Params, float RewindToTime,
	AActor* Instigator)
{
	RewindAllTargets(RewindToTime, Instigator);
	bool bHit = GetWorld()->SweepSingleByChannel(
	   OutHit, Start, End, FQuat::Identity, Channel, Shape, Params);
	RestoreAllTargets();
	return bHit;
}
