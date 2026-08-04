// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/AI/Services/M_HasTargetThirtyPercentHealth.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UM_HasTargetThirtyPercentHealth::UM_HasTargetThirtyPercentHealth()
{
    NodeName = TEXT("Check Target Health");
    Interval = 0.5f;
    RandomDeviation = 0.1f;
}

void UM_HasTargetThirtyPercentHealth::TickNode(UBehaviorTreeComponent &OwnerComp, uint8 *NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent *BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return;

    AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!IsValid(TargetActor))
    {
        BlackboardComp->SetValueAsBool(LowHealthFlagKey.SelectedKeyName, false);
        return;
    }

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
    {
        BlackboardComp->SetValueAsBool(LowHealthFlagKey.SelectedKeyName, false);
        return;
    }

    float CurrentHealth = TargetASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    float MaxHealth = TargetASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());

    bool bIsLowHealth = (MaxHealth > 0.f) && (CurrentHealth / MaxHealth <= 0.30f);

    BlackboardComp->SetValueAsBool(LowHealthFlagKey.SelectedKeyName, bIsLowHealth);
}