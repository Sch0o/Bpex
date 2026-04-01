// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/SeekerArrow.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemGlobals.h" // 用于获取 ASC
#include "AbilitySystemComponent.h"
#include "TimerManager.h"
#include "Engine/OverlapResult.h"


ASeekerArrow::ASeekerArrow()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 根据你的项目设置
	RootComponent = CollisionComp;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 模型不参与碰撞计算

	// 3. 初始化投射物运动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 箭头永远朝向飞行方向
	ProjectileMovement->bShouldBounce = false;

	// 绑定停止事件
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ASeekerArrow::OnProjectileStop);

	bReplicates = true;
}

void ASeekerArrow::BeginPlay()
{
	Super::BeginPlay();
}

void ASeekerArrow::OnProjectileStop(const FHitResult& ImpactResult)
{
	//停止后不再碰撞
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ImpactResult.GetComponent())
	{
		AttachToComponent(ImpactResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}

	GetWorldTimerManager().SetTimer(ScanTimerHandle, this, &ASeekerArrow::ExecuteScan, ScanDelay, false);
}

void ASeekerArrow::ExecuteScan()
{
	UE_LOG(LogTemp, Warning, TEXT("ASeekerArrow::ExecuteScan"));
	if (!RevealEffectClass) return;
	FVector StartLocation = GetActorLocation();
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ScanRadius);
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetInstigator());

	bool Hit = GetWorld()->OverlapMultiByChannel(OverlapResults, StartLocation, FQuat::Identity, ECC_Pawn, SphereShape,
	                                             QueryParams);
	if (Hit)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASeekerArrow Hit overlap"));
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* TargetActor = Result.GetActor();
			if (!TargetActor) continue;
			FVector TargetLocation = TargetActor->GetTargetLocation();
			FHitResult LineTraceResult;

			bool bHitObstacle = GetWorld()->LineTraceSingleByChannel(LineTraceResult, StartLocation, TargetLocation,
			                                                         ECC_Visibility, QueryParams);

			if (bHitObstacle && LineTraceResult.GetActor() != TargetActor)
			{
				continue;
			}

			UE_LOG(LogTemp, Warning, TEXT("ASeekerArrow Hit LineTraceResult"));
			UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
			if (!TargetASC) continue;
			UE_LOG(LogTemp, Warning, TEXT("ASeekerArrow no gas"));
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddInstigator(GetInstigator(), this);
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(RevealEffectClass, 1.0f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("ASeekerArrow spec"));
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

// Called every frame
void ASeekerArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
