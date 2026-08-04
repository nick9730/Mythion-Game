// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTDecorator.h"
#include "CoreMinimal.h"
#include "M_IsBlocking.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_IsBlocking : public UBTDecorator
{
    GENERATED_BODY()

  public:
    UM_IsBlocking();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector TargetKey;

    //   virtual bool CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const override;
};
