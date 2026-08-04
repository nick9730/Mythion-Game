// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/Enemies/AI/Decorators/M_IsTheTargetInCloseDistance.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemy.h"
#include "GameFramework/Actor.h"

UM_IsTheTargetInCloseDistance::UM_IsTheTargetInCloseDistance()
{
    NodeName = TEXT("Target Is At Safe Distance");
}

bool UM_IsTheTargetInCloseDistance::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp,
                                                               uint8 *NodeMemory) const
{
    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return false;

    AAIController *AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
        return false;

    AEnemy *EnemyPawn = Cast<AEnemy>(AIController->GetPawn());
    if (!IsValid(EnemyPawn))
        return false;

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return false;

    float Distance = FVector::Dist(EnemyPawn->GetActorLocation(), TargetActor->GetActorLocation());

    return Distance <= SafeDistanceForBlink;
}