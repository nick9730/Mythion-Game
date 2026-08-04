// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "M_FindPatrolPosition.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_FindPatrolPosition : public UBTTaskNode
{
    GENERATED_BODY()

  public:
    UM_FindPatrolPosition();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector HomeLocationKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector PatrolLocationKey;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float Radius = 1000.f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;
};
