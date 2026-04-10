#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ShooterEnemyDummy.generated.h"

struct FOnAttributeChangeData;
class UGameplayEffect;

UCLASS()
class BPEX_API AShooterEnemyDummy : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AShooterEnemyDummy();
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    class UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    class UBpexAttributeSet* AttributeSet;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

    void InitializePrimaryAttributes() const;
    virtual void HealthChanged(const FOnAttributeChangeData& Data);

    bool bIsDead = false;
    FTimerHandle DeathTimerHandle;
    void Die();
    void Disappear();

    //══════════════════════════════════════
    // ★ 左右摇摆 ★
    // ══════════════════════════════════════

    /** 是否启用左右摇摆 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Strafe")
    bool bEnableStrafe = true;

    /** 左右摇摆的总宽度（单位：厘米，单侧距离的2倍） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Strafe",meta = (ClampMin = "0", EditCondition = "bEnableStrafe"))
    float StrafeDistance = 1000.f;

    /** 一个完整来回周期的时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Strafe",
              meta = (ClampMin = "0.1", EditCondition = "bEnableStrafe"))
    float StrafePeriod = 2.0f;

    /** 摇摆方向：Local Right（可在编辑器旋转Actor改变方向） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Strafe",
              meta = (EditCondition = "bEnableStrafe"))
    bool bStrafeAlongRight = true;

    /** 出生点（摇摆中心） */
    FVector SpawnLocation;

    /** 摇摆方向轴（归一化） */
    FVector StrafeAxis;

    /** 累计时间 */
    float StrafeTimer = 0.f;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};