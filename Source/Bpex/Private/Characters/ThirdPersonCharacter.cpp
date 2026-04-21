// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ThidPerson/ThirdPersonCharacter.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "AbilitySystem/BpexAbilitySystemComponent.h"
#include "Input/BpexCharacterMovementComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AThirdPersonCharacter::AThirdPersonCharacter(const FObjectInitializer& ObjectInitializer): Super(
	ObjectInitializer.SetDefaultSubobjectClass<UBpexCharacterMovementComponent>(
		CharacterMovementComponentName))
{
	
	PrimaryActorTick.bCanEverTick = true;

	// ----------------- 1. TPS 专属的旋转规则 -----------------
	// 解绑角色与控制器的旋转：不要让人物模型跟着相机的朝向强行转动（这是 FPS 的做法）
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// 配置移动组件：让人物模型在移动时，自动转身面向前进的方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 设置转身的平滑速度
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// ----------------- 2. 创建弹簧臂 -----------------
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); // 绑定到角色的根组件（胶囊体）上
	CameraBoom->TargetArmLength = 400.0f;       // 设置相机跟在人物背后的距离
	CameraBoom->bUsePawnControlRotation = true; // 【最关键一步】让弹簧臂跟随鼠标（控制器）旋转！

	// ----------------- 3. 创建摄像机 -----------------
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 把相机挂在弹簧臂的末端
	FollowCamera->bUsePawnControlRotation = false;
	
	// ----------------- 4. 创建碰撞盒 -----------------
	HitBox_Head       = CreateHitBox("HitBox_Head",       "head",       FVector(12, 12, 12), FVector(0, 0, 5));
	HitBox_Neck       = CreateHitBox("HitBox_Neck",       "neck_01",    FVector(8,  8,  6));
	HitBox_Chest      = CreateHitBox("HitBox_Chest",      "spine_05",   FVector(20, 28, 18));
	HitBox_Abdomen    = CreateHitBox("HitBox_Abdomen",    "spine_01",   FVector(18, 25, 15));
	HitBox_UpperArm_L = CreateHitBox("HitBox_UpperArm_L", "upperarm_l", FVector(15, 7,  7));
	HitBox_LowerArm_L = CreateHitBox("HitBox_LowerArm_L", "lowerarm_l", FVector(14, 6,  6));
	HitBox_UpperArm_R = CreateHitBox("HitBox_UpperArm_R", "upperarm_r", FVector(15, 7,  7));
	HitBox_LowerArm_R = CreateHitBox("HitBox_LowerArm_R", "lowerarm_r", FVector(14, 6,  6));
	HitBox_Thigh_L    = CreateHitBox("HitBox_Thigh_L",    "thigh_l",    FVector(8,  8,  22));
	HitBox_Calf_L     = CreateHitBox("HitBox_Calf_L",     "calf_l",     FVector(7,  7,  20));
	HitBox_Thigh_R    = CreateHitBox("HitBox_Thigh_R",    "thigh_r",    FVector(8,  8,  22));
	HitBox_Calf_R     = CreateHitBox("HitBox_Calf_R",     "calf_r",     FVector(7,  7,  20));

}


UAbilitySystemComponent* AThirdPersonCharacter::GetAbilitySystemComponent() const
{
	return Cast<UBpexAbilitySystemComponent>(AbilitySystemComponent);
}

UAttributeSet* AThirdPersonCharacter::CreateAttributeSet()
{
	return AttributeSet;
}

// Called when the game starts or when spawned
void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AThirdPersonCharacter::MoveInput(const FInputActionValue& Value)
{

	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AThirdPersonCharacter::InitAbilityActorInfo()
{
}

void AThirdPersonCharacter::InitializePrimaryAttributes() const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(DefaultPrimaryAttributes);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		DefaultPrimaryAttributes, 1, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AThirdPersonCharacter::AddCharacterAbilities()
{
	if (!HasAuthority()) return;
	UBpexAbilitySystemComponent* BpexASC = CastChecked<UBpexAbilitySystemComponent>(AbilitySystemComponent);

	BpexASC->AddCharacterAbilities(StartupAbilities);
}

// Called every frame
void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::LookInput);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AThirdPersonCharacter::DoJumpEnd);
	}

}

UBpexCharacterMovementComponent* AThirdPersonCharacter::GetBpexCharacterMovementComponent() const
{
	return Cast<UBpexCharacterMovementComponent>(GetCharacterMovement());
}

UCameraComponent* AThirdPersonCharacter::GetCameraComponent() const
{
	return FollowCamera;
}

void AThirdPersonCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);   // 左右看
		AddControllerPitchInput(LookAxisVector.Y); // 上下看
	}
}

void AThirdPersonCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AThirdPersonCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

UBoxComponent* AThirdPersonCharacter::CreateHitBox(FName CompName, FName BoneName, FVector Extent, FVector Offset)
{
	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(CompName);
	// 设置大小
	Box->SetBoxExtent(Extent);
	// Attach到骨骼
	Box->SetupAttachment(GetMesh(), BoneName);
	// 设置相对偏移
	Box->SetRelativeLocation(Offset);
	// ===== 碰撞设置：不参与物理，仅用于延迟补偿 =====
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetGenerateOverlapEvents(false);
	Box->SetCanEverAffectNavigation(false);
	// 编辑器中可见（线框显示）
	Box->SetHiddenInGame(true);         // 游戏中隐藏
	Box->bVisualizeComponent = true;     // 编辑器中显示线框
	return Box;
}


