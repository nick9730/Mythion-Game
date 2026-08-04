// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Tasks/M_ClearFocus.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UM_ClearFocus::UM_ClearFocus()
{
    NodeName = TEXT("UnFocus On Target");
}

EBTNodeResult::Type UM_ClearFocus::ExecuteTask(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory)
{
    AAIController *AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
        return EBTNodeResult::Failed;

    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return EBTNodeResult::Failed;

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return EBTNodeResult::Failed;

    AIController->ClearFocus(EAIFocusPriority::Gameplay);

    return EBTNodeResult::Succeeded;
}