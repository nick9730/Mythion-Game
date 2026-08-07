// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayEffectTypes.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "M_CalculationExecution.generated.h"

/**
 *
 */
class AActor;
class UAbilitySystemComponent;

UCLASS()
class MYTHION_API UM_CalculationExecution : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters &ExecutionParams,
                                        FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const override;

    void BuildGameplayEvent(FGameplayEventData &EventData, FName EventTag, AActor *Instigator, AActor *Target) const;

    void ApplyPhysicalDamage(float &FinalDamage, UAbilitySystemComponent *TargetASC, AActor *SourceActor,
                             AActor *TargetActor, FGameplayCueParameters CueParams, FGameplayEventData EventData,
                             FGameplayEventData EventForPlayerHit,
                             const FGameplayEffectCustomExecutionParameters &ExecutionParams, bool bIsFrontalHit,
                             FGameplayTag IsBlockingTag) const;

    void ApplyMagicalDamage(float &FinalDamage, UAbilitySystemComponent *TargetASC, bool bIsFrontalHit,
                            FGameplayTag IsBlockingTag) const;

    void FindAndSetIsFrontalHit(AActor *TargetActor, AActor *AttackerActor, bool &bOutIsFrontalHit) const;
};