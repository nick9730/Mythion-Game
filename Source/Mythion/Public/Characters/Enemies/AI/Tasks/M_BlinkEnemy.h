// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "M_BlinkEnemy.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_BlinkEnemy : public UBTTaskNode
{
    GENERATED_BODY()
    UM_BlinkEnemy();

  public:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "Abilities")
    FGameplayTag Blink;
};
