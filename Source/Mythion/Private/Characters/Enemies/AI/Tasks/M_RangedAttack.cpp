// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Tasks/M_RangedAttack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemy.h"
#include "GameFramework/Actor.h"

UM_RangedAttack::UM_RangedAttack()
{
    NodeName = TEXT("Ranged Attack");
}

EBTNodeResult::Type UM_RangedAttack::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
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

    bool bHasThirtyPercentOfHealth = BlackboardComp->GetValueAsBool(HasThirtyPercentOfHealth.SelectedKeyName);

    FGameplayTagContainer TagContainer;

    if (bHasThirtyPercentOfHealth)
    {
        TagContainer.AddTag(FirstAbility);
    }
    else
    {
        TagContainer.AddTag(SecondAbility);
    }

    ASC->TryActivateAbilitiesByTag(TagContainer);

    return EBTNodeResult::Succeeded;
}