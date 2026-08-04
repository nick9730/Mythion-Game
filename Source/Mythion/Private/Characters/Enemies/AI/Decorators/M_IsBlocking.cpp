// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Decorators/M_IsBlocking.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "GameFramework/Actor.h"

UM_IsBlocking::UM_IsBlocking()
{
    NodeName = TEXT("Target Is Blocking");
}

/*
{
bool UM_IsBlocking::CalculateRawConditionValue(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory) const
     UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
     if (!IsValid(BlackboardComp))
     return false;

     AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
     if (!IsValid(TargetActor))
     return false;

     UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
     if (!IsValid(TargetASC))
     return false;

     FGameplayTag BlockingTag = FGameplayTag::RequestGameplayTag(FName("Status.IsBlocking"));

     return TargetASC->HasMatchingGameplayTag(BlockingTag);
    }
    */