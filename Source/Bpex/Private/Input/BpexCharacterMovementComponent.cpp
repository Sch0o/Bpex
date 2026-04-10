// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/BpexCharacterMovementComponent.h"
#include "Characters/ThidPerson/ApexCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"


UBpexCharacterMovementComponent::UBpexCharacterMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	NavAgentProps.bCanCrouch = true;
}

//check current move and next move ,can we combine to save bandwidth
bool UBpexCharacterMovementComponent::FSavedMove_Bpex::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                      ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Bpex* NewBpexMove = static_cast<FSavedMove_Bpex*>(NewMove.Get());

	if (Saved_bWantsToSprint != NewBpexMove->Saved_bWantsToSprint)
	{
		return false;
	}

	if (Saved_bWantsToSlide != NewBpexMove->Saved_bWantsToSlide)
	{
		return false;
	}
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UBpexCharacterMovementComponent::FSavedMove_Bpex::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;
	Saved_bWantsToSlide = 0;
}

uint8 UBpexCharacterMovementComponent::FSavedMove_Bpex::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (Saved_bWantsToSprint)
		Result |= FLAG_Custom_0;
	if (Saved_bWantsToSlide)
		Result |= FLAG_Custom_1;
	return Result;
}

void UBpexCharacterMovementComponent::FSavedMove_Bpex::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                  FVector const& NewAccel,
                                                                  class FNetworkPredictionData_Client_Character&
                                                                  ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UBpexCharacterMovementComponent* CharacterMovement = Cast<UBpexCharacterMovementComponent>(
		C->GetCharacterMovement());
	Saved_bWantsToSprint = CharacterMovement->bSafe_WantsToSprint;
	Saved_bWantsToSlide = CharacterMovement->bSafe_WantsToSlide;
}

void UBpexCharacterMovementComponent::FSavedMove_Bpex::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UBpexCharacterMovementComponent* CharacterMovement = Cast<UBpexCharacterMovementComponent>(
		C->GetCharacterMovement());

	CharacterMovement->bSafe_WantsToSprint = Saved_bWantsToSprint;
	CharacterMovement->bSafe_WantsToSlide = Saved_bWantsToSlide;
}

UBpexCharacterMovementComponent::FNetWorkPredictionData_Client_Bpex::FNetWorkPredictionData_Client_Bpex(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UBpexCharacterMovementComponent::FNetWorkPredictionData_Client_Bpex::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Bpex());
}

void UBpexCharacterMovementComponent::EnterSlide()
{
	UE_LOG(LogTemp,Warning,TEXT("Entering slide"));
	Crouch();
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
	
	if (ApexCharacterOwner->HasAuthority())
	{
		ApexCharacterOwner->bIsSliding = true;
	}
}

void UBpexCharacterMovementComponent::ExitSlide()
{
	SetMovementMode(MOVE_Walking);
	
	if (ApexCharacterOwner->HasAuthority())
	{
		ApexCharacterOwner->bIsSliding = false;
	}
}

void UBpexCharacterMovementComponent::EnterSprint()
{
	bSafe_WantsToSprint = true;
}

void UBpexCharacterMovementComponent::ExitSprint()
{
	bSafe_WantsToSprint = false;
}

bool UBpexCharacterMovementComponent::IsSliding() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Slide)
	{
		return true;
	}
		return false;
}

FNetworkPredictionData_Client* UBpexCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner!=nullptr)

	//懒加载单例模式
	if (ClientPredictionData == nullptr)
	{
		UBpexCharacterMovementComponent* MutableThis = const_cast<UBpexCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetWorkPredictionData_Client_Bpex(*this);
		//当服务器修正你的位置且偏差<=92cm时，平滑插值
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		//>=某值时不再尝试平滑过渡，直接瞬移
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UBpexCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bSafe_WantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bSafe_WantsToSlide = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void UBpexCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBpexCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ApexCharacterOwner = Cast<AApexCharacter>(GetOwner());

	if (!ApexCharacterOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("BpexMovementComponent 必须挂载在 AApexCharacter 及其子类上！"));
	}
}

void UBpexCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		return;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Unknown Movement Mode"));
	}
}

void UBpexCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
                                                        const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking)
	{
		if (bSafe_WantsToSprint)
		{
			MaxWalkSpeed = Spring_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}

void UBpexCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaTime)
{
	//处理滑铲进入
	 if (bSafe_WantsToSlide)
	 {
	 	bWantsToCrouch = true;
	 	if (Velocity.Size2D()>=MinSlideSpeed&&MovementMode == MOVE_Walking)
	 	{
	 		EnterSlide();
	 	}
	 }else
	 {
	 	bWantsToCrouch = false;
	 	if (IsSliding())
	 	{
	 		ExitSlide();
	 	}
	 }
	 Super::UpdateCharacterStateBeforeMovement(DeltaTime);

	
}

bool UBpexCharacterMovementComponent::CanCrouchInCurrentState() const
{
	if (IsSliding())
	{
		return true;
	}
	return Super::CanCrouchInCurrentState();
}

bool UBpexCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround()||IsSliding();
}

void UBpexCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) return;

    // 获取当前的网络身份，方便看 Log 时区分是哪一端退出的
    FString RoleStr = CharacterOwner && CharacterOwner->HasAuthority() ? TEXT("[Server]") : TEXT("[Client]");

    FHitResult HitResult;
    
    // ==========================================
    // 退出点 1：射线检测没有打到地面 (悬空了)
    // ==========================================
    if (!GetSlideSurface(HitResult))
    {
       UE_LOG(LogTemp, Warning, TEXT("%s 🛑 滑铲退出 1：GetSlideSurface 失败，角色离开了地面！"), *RoleStr);
       ExitSlide();
       StartNewPhysics(deltaTime, Iterations + 1);
       return;
    }

    // ==========================================
    // 退出点 2：速度衰减到了阈值以下
    // ==========================================
    if (Velocity.Size2D() < MinSlideSpeed)
    {
       UE_LOG(LogTemp, Warning, TEXT("%s 🛑 滑铲退出 2：速度过低！当前速度: %.1f，最小要求: %.1f"), *RoleStr, Velocity.Size2D(), MinSlideSpeed);
       ExitSlide();
       StartNewPhysics(deltaTime, Iterations + 1);
       return;
    }

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    MaintainHorizontalGroundVelocity();
    
    if (CurrentFloor.bBlockingHit)
    {
       FVector SlopeForce = CurrentFloor.HitResult.Normal;
       SlopeForce.Z = 0.f;
       Velocity += SlopeForce * SlideGravityForce * SlideGravityModifier * deltaTime;
    }
    
    Acceleration = FVector::ZeroVector;
    CalcVelocity(deltaTime, SlideFriction, false, GetMaxBrakingDeceleration());
    FVector MoveVelocity = Velocity;
    FVector Delta = deltaTime * MoveVelocity;
    
    // ==========================================
    // 退出点 3：本帧位移几乎为 0 (通常是撞到了墙或者死角卡住了)
    // ==========================================
    if (Delta.IsNearlyZero())
    {
       UE_LOG(LogTemp, Warning, TEXT("%s 🛑 滑铲退出 3：Delta位移几乎为0，角色撞墙或完全停下！"), *RoleStr);
       ExitSlide();
       return;
    }
    
    FStepDownResult StepDownResult;
    MoveAlongFloor(MoveVelocity, deltaTime, &StepDownResult);
    
    // ==========================================
    // 退出点 4：引擎原生判定进入了掉落状态
    // ==========================================
    if (IsFalling())
    {
       UE_LOG(LogTemp, Warning, TEXT("%s 🛑 滑铲退出 4：原生 IsFalling() 判定悬空 (走出了悬崖边缘)！"), *RoleStr);
       ExitSlide(); 
       StartNewPhysics(deltaTime, Iterations + 1);
       return;
    }
    
    if (StepDownResult.bComputedFloor)
    {
       CurrentFloor = StepDownResult.FloorResult;
    }
    else
    {
       FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, Delta.IsNearlyZero(), NULL);
    }
    
    // ==========================================
    // 退出点 5：滑到了不可行走的表面 (比如非常陡峭的斜坡/墙根)
    // ==========================================
    if (CurrentFloor.IsWalkableFloor())
    {
       AdjustFloorHeight();
       SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
    }
    else
    {
       UE_LOG(LogTemp, Warning, TEXT("%s 🛑 滑铲退出 5：地板变为不可行走 (IsWalkableFloor = false)！地板法线Z: %.2f"), *RoleStr, CurrentFloor.HitResult.Normal.Z);
       ExitSlide();
    }
    
    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
       //Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
       MaintainHorizontalGroundVelocity(); // 再次确保 Z 轴速度是干净的 0
    }
	
}

bool UBpexCharacterMovementComponent::GetSlideSurface(FHitResult& HitResult) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + FVector::DownVector * 120.f;
	//FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");

	return GetWorld()->LineTraceSingleByProfile(HitResult, Start, End, ProfileName,
	                                            ApexCharacterOwner->GetIgnoreCharacterParams());
}


void UBpexCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                    FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UBpexCharacterMovementComponent::GetMinSlideSpeed() const
{
	return MinSlideSpeed;
}
