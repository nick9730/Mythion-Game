// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Tasks/M_AttackThePlayer.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemy.h"
#include "GameFramework/Actor.h"

UM_AttackThePlayer::UM_AttackThePlayer()
{
    NodeName = TEXT("Attack The Target");
}

EBTNodeResult::Type UM_AttackThePlayer::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
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

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return EBTNodeResult::Failed;

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(EnemyPawn->MeleeAttackAbilityTag);
    ASC->TryActivateAbilitiesByTag(TagContainer);

    return EBTNodeResult::Succeeded;
}
