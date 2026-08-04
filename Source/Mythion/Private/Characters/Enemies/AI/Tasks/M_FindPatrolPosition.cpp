// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Tasks/M_FindPatrolPosition.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UM_FindPatrolPosition::UM_FindPatrolPosition()
{
    NodeName = TEXT("Find Patrol Position");
}

EBTNodeResult::Type UM_FindPatrolPosition::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return EBTNodeResult::Failed;

    FVector HomeLocation = BlackboardComp->GetValueAsVector(HomeLocationKey.SelectedKeyName);

    UNavigationSystemV1 *NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem))
        return EBTNodeResult::Failed;

    FNavLocation ResultLocation;
    bool bFound = NavSystem->GetRandomReachablePointInRadius(HomeLocation, Radius, ResultLocation);
    if (!bFound)
        return EBTNodeResult::Failed;

    BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, ResultLocation.Location);

    return EBTNodeResult::Succeeded;
}