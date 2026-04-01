// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/GA_FireSeekerArrow.h"
#include "Weapon/SeekerArrow.h"
#include "GameFramework/Character.h"
#include "InventorySystem/Types/InventoryTypes.h"
#include "Weapon/CombatComponent.h"
#include "Weapon/ShooterWeapon.h"

UGA_FireSeekerArrow::UGA_FireSeekerArrow()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_FireSeekerArrow::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("==== 寻敌箭技能正式开始 Activate ===="));
	if (!CommitAbility(Handle,ActorInfo,ActivationInfo,nullptr))
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	ACharacter*AvatarCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!AvatarCharacter||!ArrowClass)
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	UCombatComponent* CombatComponent = AvatarCharacter->FindComponentByClass<UCombatComponent>();
	if (!CombatComponent||!CombatComponent->GetCurrentWeapon())
	{
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	if (AvatarCharacter->HasAuthority())
	{
		FVector TargetLocation = CombatComponent->GetWeaponTargetLocation();
		FTransform SpawnTransform = CombatComponent->GetCurrentWeapon()->CalculateProjectileSpawnTransform(TargetLocation);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = AvatarCharacter;
		SpawnParams.Owner = AvatarCharacter;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // 即使有点穿模也强制生成

		ASeekerArrow* SpawnedArrow = GetWorld()->SpawnActor<ASeekerArrow>(ArrowClass, SpawnTransform, SpawnParams);
	}
	EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}
