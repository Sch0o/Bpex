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
	 // FHitResult SurfaceHit = CurrentFloor.HitResult;
	//  if (CurrentFloor.IsWalkableFloor())
	//  {
	//  	Velocity = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal);
	//  }
	// Velocity.Z -= 500.f;
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
	
	// if (bSafe_WantsToSlide) bWantsToCrouch = true;
	// else { bWantsToCrouch = false; if (IsSliding()) ExitSlide(); }
	//
	// // 【务必放在判断 EnterSlide 的前面！】让原生逻辑先把脚踩实！
	// Super::UpdateCharacterStateBeforeMovement(DeltaTime); 
	//
	// if (bSafe_WantsToSlide && bWantsToCrouch)
	// {
	// 	if (MovementMode == MOVE_Walking && Velocity.Size2D() >= MinSlideSpeed)
	// 	{
	// 		EnterSlide(); // 在这里才切入 Custom 模式
	// 	}
	// }
	
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

	// FVector Gravity = FVector(0.f, 0.f, GetGravityZ());
	// FVector SlopeAcceleration = FVector::VectorPlaneProject(Gravity, HitResult.Normal);

	// Velocity += SlopeAcceleration * deltaTime * SlideGravityModifier;
	// Velocity.Z += (Gravity.Z) * deltaTime;
	// 摩擦力
	//Velocity = Velocity - Velocity * SlideFriction * deltaTime;
	// Acceleration = FVector::ZeroVector;
	// CalcVelocity(deltaTime, SlideFriction, true, GetMaxBrakingDeceleration());
	//
	// FVector Adjusted = Velocity * deltaTime;
	// FHitResult Hit(1.f);
	// SafeMoveUpdatedComponent(Adjusted, CharacterOwner->GetActorRotation(), true, Hit);
	//
	// if (Hit.Time < 1.f)
	// {
	// 	HandleImpact(Hit, deltaTime, Adjusted);
	// 	SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	// }
	// if (UpdatedComponent)
	// {
	// 	FVector SnapStart = UpdatedComponent->GetComponentLocation();
	// 	// 往下扫模的距离。建议 50~60 左右（相当于能容忍的最大台阶/下坡落差）
	// 	float SnapDistance = 60.f; 
	// 	FVector SnapEnd = SnapStart + FVector::DownVector * SnapDistance;
	//
	// 	FHitResult SnapHit;
	// 	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SlideSnap), false, CharacterOwner);
	//
	// 	// 使用当前胶囊体的大小往下“压”一个形状，而不是用细线去Trace（细线容易漏过裂缝）
	// 	bool bHitFloor = GetWorld()->SweepSingleByChannel(
	// 		SnapHit, 
	// 		SnapStart, 
	// 		SnapEnd, 
	// 		FQuat::Identity, 
	// 		UpdatedComponent->GetCollisionObjectType(), // 角色当前的碰撞通道
	// 		GetPawnCapsuleCollisionShape(SHRINK_None),  // 保持当前下蹲胶囊体尺寸
	// 		QueryParams
	// 	);
	//
	// 	// 如果扫到了地板，且地板是一个有效的阻挡，且当前不是卡在地板里（Distance > 0）
	// 	if (bHitFloor && SnapHit.IsValidBlockingHit() && SnapHit.Distance > 0.f)
	// 	{
	// 		// 判断这个坡度是不是太陡了，防止吸附到悬崖壁上（可选，根据你的游戏需求）
	// 		if (IsWalkable(SnapHit)) 
	// 		{
	// 			// 计算出恰好贴地的向下偏移量
	// 			FVector SnapAdjustment = FVector(0.f, 0.f, -SnapHit.Distance);
	// 			FHitResult MoveHit;
	// 			
	// 			// 强行把角色往下拉！因为用了 SafeMoveUpdatedComponent，绝对不会穿模
	// 			SafeMoveUpdatedComponent(SnapAdjustment, CharacterOwner->GetActorRotation(), true, MoveHit);
	// 		}
	// 	}
	// }
	//
	// FHitResult NewHitResult;
	// if (!GetSlideSurface(NewHitResult) || Velocity.Size2D() < MinSlideSpeed)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("不在地面 or 速度：%f 过小" ), Velocity.Size2D());
	// 	ExitSlide();
	// }
	//
	// if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	// {
	// 	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	// }
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
	
	if (CharacterOwner)
	{
		// 1. 获取当前的网络身份，方便区分是谁打印的
		FString RoleStr;
		if (CharacterOwner->HasAuthority())
		{
			RoleStr = TEXT("[Server]");
		}
		else if (CharacterOwner->IsLocallyControlled())
		{
			RoleStr = TEXT("[Local Client]");
		}
		else
		{
			RoleStr = TEXT("[Simulated Proxy]");
		}

		// 2. 格式化速度字符串，包含 X, Y, Z 以及水平总速度(Size2D)
		FString VelocityMsg = FString::Printf(TEXT("%s Vel X: %.1f | Y: %.1f | Z: %.1f || Speed2D: %.1f"), 
			*RoleStr, 
			Velocity.X, 
			Velocity.Y, 
			Velocity.Z, 
			Velocity.Size2D());

		// 3. 打印到屏幕上
		if (GEngine)
		{
			// 参数解释：
			// Key: 分配一个唯一的整型ID（比如根据Role给不同ID）。同一个ID的文本会实时覆写，而不会刷屏！
			// TimeToDisplay: 0.0f 表示只显示一帧
			// Color: 颜色
			int32 MsgKey = CharacterOwner->HasAuthority() ? 100 : (CharacterOwner->IsLocallyControlled() ? 101 : 102);
			FColor MsgColor = CharacterOwner->HasAuthority() ? FColor::Cyan : FColor::Green;
        
			GEngine->AddOnScreenDebugMessage(MsgKey, 0.0f, MsgColor, VelocityMsg);
		}
	}
}

float UBpexCharacterMovementComponent::GetMinSlideSpeed() const
{
	return MinSlideSpeed;
}
