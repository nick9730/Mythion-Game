// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "CoreMinimal.h"
#include "M_IsTheTargetInCloseDistance.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_IsTheTargetInCloseDistance : public UBTDecorator
{
    GENERATED_BODY()

  public:
    UM_IsTheTargetInCloseDistance();

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    float SafeDistanceForBlink = 500.f;

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector TargetKey;

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const override;
};
