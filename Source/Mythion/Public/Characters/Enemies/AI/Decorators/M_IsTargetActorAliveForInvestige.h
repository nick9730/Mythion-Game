// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "CoreMinimal.h"
#include "M_IsTargetActorAliveForInvestige.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_IsTargetActorAliveForInvestige : public UBTDecorator
{
    GENERATED_BODY()

  public:
    UM_IsTargetActorAliveForInvestige();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector InvestigateTargetKey;

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const override;
};
