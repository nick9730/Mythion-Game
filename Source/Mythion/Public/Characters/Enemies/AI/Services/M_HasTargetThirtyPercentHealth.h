// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTService.h"
#include "CoreMinimal.h"
#include "M_HasTargetThirtyPercentHealth.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_HasTargetThirtyPercentHealth : public UBTService
{
    GENERATED_BODY()

  public:
    UM_HasTargetThirtyPercentHealth();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector TargetKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector LowHealthFlagKey;

    virtual void TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds) override;
};
