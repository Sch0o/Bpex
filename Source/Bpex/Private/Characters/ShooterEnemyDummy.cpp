// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterEnemyDummy.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/BpexAttributeSet.h"


// Sets default values
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

// Called when the game starts or when spawned
void AShooterEnemyDummy::BeginPlay()
{
	Super::BeginPlay();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this,this);
		InitializePrimaryAttributes();
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UBpexAttributeSet::GetHealthAttribute()
		).AddUObject(this, &AShooterEnemyDummy::HealthChanged);
	}
} 

void AShooterEnemyDummy::InitializePrimaryAttributes() const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(DefaultPrimaryAttributes);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		DefaultPrimaryAttributes, 1, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AShooterEnemyDummy::HealthChanged(const FOnAttributeChangeData& Data)
{
	float NewHealth = Data.NewValue;
	float OldHealth = Data.OldValue;
	float DamageTaken = OldHealth - NewHealth; // 计算这一下扣了多少血

	// 1. 在左上角屏幕打印 (仅在编辑器和开发版本中显示)
	if (GEngine)
	{
		// 格式化我们要输出的文字
		FString DebugMsg = FString::Printf(TEXT("木桩挨打了！受到伤害: %.1f | 当前血量: %.1f"), DamageTaken, NewHealth);
		
		// -1 代表不需要覆盖上一条消息，2.0f 是停留两秒，红色字体
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
	}

	// 2. 顺便在底部的 Output Log（输出日志）里也打印一份，方便查阅
	UE_LOG(LogTemp, Warning, TEXT("Dummy Health: %f -> %f"), OldHealth, NewHealth);
}

// Called every frame
void AShooterEnemyDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AShooterEnemyDummy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

