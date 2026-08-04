// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "M_RangedAttack.generated.h"

UCLASS()
class MYTHION_API UM_RangedAttack : public UBTTaskNode
{
    GENERATED_BODY()

  public:
    UM_RangedAttack();

  protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    struct FBlackboardKeySelector HasThirtyPercentOfHealth;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) override;

    UPROPERTY(EditAnywhere, Category = "Abilities")
    FGameplayTag FirstAbility;

    UPROPERTY(EditAnywhere, Category = "Abilities")
    FGameplayTag SecondAbility;
};