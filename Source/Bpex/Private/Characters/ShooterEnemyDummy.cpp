#include "Characters/ShooterEnemyDummy.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/BpexAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

AShooterEnemyDummy::AShooterEnemyDummy()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AttributeSet = CreateDefaultSubobject<UBpexAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AShooterEnemyDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AShooterEnemyDummy::BeginPlay()
{
	Super::BeginPlay();

	//── GAS 初始化 ──
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		InitializePrimaryAttributes();
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UBpexAttributeSet::GetHealthAttribute()
		).AddUObject(this, &AShooterEnemyDummy::HealthChanged);
	}

	// ──摇摆初始化 ──
	SpawnLocation = GetActorLocation();
	StrafeAxis = bStrafeAlongRight
		             ? GetActorRightVector()
		             : GetActorForwardVector();

	// 随机起始相位，多个木桩不会同步摇
	StrafeTimer = FMath::FRandRange(0.f, StrafePeriod);
}

void AShooterEnemyDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ── 左右摇摆（仅服务器驱动，CharacterMovement 自动同步） ──
	if (bEnableStrafe && !bIsDead && HasAuthority())
	{
		StrafeTimer += DeltaTime;

		// sin 产生 -1 ~ +1 的平滑往复
		const float Alpha = FMath::Sin(StrafeTimer * (2.f * PI) / StrafePeriod);

		// 目标位置 = 出生点 ± 半幅
		const float HalfDist = StrafeDistance * 0.5f;
		const FVector TargetLocation = SpawnLocation + StrafeAxis * Alpha * HalfDist;

		// 用AddMovementInput 驱动 CharacterMovement（有加速/减速，更自然）
		const FVector ToTarget = TargetLocation - GetActorLocation();
		const float DistToTarget = ToTarget.Size();

		if (DistToTarget > 1.f)
		{
			const FVector MoveDir = ToTarget.GetSafeNormal();
			// 越接近目标越慢（平滑停止）
			const float SpeedFactor = FMath::Clamp(DistToTarget / (HalfDist * 0.5f), 0.1f, 1.f);
			AddMovementInput(MoveDir, SpeedFactor);
		}
	}
}

//══════════════════════════════════════════════════════
// 以下和原来完全一样
// ══════════════════════════════════════════════════════

void AShooterEnemyDummy::InitializePrimaryAttributes() const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(DefaultPrimaryAttributes);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		DefaultPrimaryAttributes, 1, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AShooterEnemyDummy::HealthChanged(const FOnAttributeChangeData& Data)
{
	float NewHealth = Data.NewValue;
	float OldHealth = Data.OldValue;

	FString Msg = FString::Printf(TEXT("[%s] Health: %.1f → %.1f (Delta: %.1f) | bIsDead: %s | Role: %s"),
	                              *GetName(),
	                              OldHealth,
	                              NewHealth,
	                              NewHealth - OldHealth,
	                              bIsDead ? TEXT("true") : TEXT("false"),
	                              *UEnum::GetValueAsString(GetLocalRole()));
	if (GEngine&&HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Msg);
	}


	if (NewHealth <= 0.0f && !bIsDead)
	{
		Die();
	}
}

void AShooterEnemyDummy::Die()
{
	bIsDead = true; // ★ 原来漏了这句
	bEnableStrafe = false; // ★ 死了不再摇

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (GetMesh())
	{
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetPhysicsBlendWeight(1.0f);
	}

	GetWorld()->GetTimerManager().SetTimer(
		DeathTimerHandle, this, &AShooterEnemyDummy::Disappear, 3.0f, false);
}

void AShooterEnemyDummy::Disappear()
{
	Destroy();
}

void AShooterEnemyDummy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
