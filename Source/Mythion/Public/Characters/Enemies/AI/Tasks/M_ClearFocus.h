// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "M_ClearFocus.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_ClearFocus : public UBTTaskNode
{
    GENERATED_BODY()

  public:
    UM_ClearFocus();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector TargetKey;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;
};
