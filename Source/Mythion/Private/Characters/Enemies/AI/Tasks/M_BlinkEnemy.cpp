// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Tasks/M_BlinkEnemy.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemy.h"
#include "GameFramework/Actor.h"

UM_BlinkEnemy::UM_BlinkEnemy()
{
    NodeName = TEXT("Blink");
}

EBTNodeResult::Type UM_BlinkEnemy::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    AAIController *AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
        return EBTNodeResult::Failed;

    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return EBTNodeResult::Failed;

    AEnemy *EnemyPawn = Cast<AEnemy>(AIController->GetPawn());
    if (!IsValid(EnemyPawn))
        return EBTNodeResult::Failed;

    UAbilitySystemComponent *ASC = EnemyPawn->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return EBTNodeResult::Failed;

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(Blink);

    ASC->TryActivateAbilitiesByTag(TagContainer);

    return EBTNodeResult::Succeeded;
}
