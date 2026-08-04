// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Decorators/M_IsTargetActorAlive.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "GameFramework/Actor.h"

UM_IsTargetActorAlive::UM_IsTargetActorAlive()
{
    NodeName = TEXT("Target Is Alive");
}

bool UM_IsTargetActorAlive::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const
{
    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return false;

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return false;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return false;

    FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Status.Dead"));

    bool bIsDead = TargetASC->HasMatchingGameplayTag(DeadTag);

    if (bIsDead)
    {
        AM_Enemy_Controller *AIController = Cast<AM_Enemy_Controller>(OwnerComp.GetAIOwner());
        if (IsValid(AIController))
        {
            AIController->SetEnemyToPassive();
        }
        return false;
    }
    return !TargetASC->HasMatchingGameplayTag(DeadTag);
}