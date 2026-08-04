// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Decorators/M_IsTargetActorAliveForInvestige.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UM_IsTargetActorAliveForInvestige::UM_IsTargetActorAliveForInvestige()
{
    NodeName = TEXT("Investigate Target Is Alive");
}

bool UM_IsTargetActorAliveForInvestige::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp,
                                                                   uint8 *NodeMemory) const
{
    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return true;

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(InvestigateTargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return true;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return true;

    FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Status.Dead"));

    return !TargetASC->HasMatchingGameplayTag(DeadTag);
}