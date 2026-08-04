// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "BehaviorTree/BlackboardComponent.h"
#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_EnemyBlink.generated.h"

/**
 *
 */

class AEnemy;
struct FBlackboardKeySelector;

UCLASS()
class MYTHION_API UGA_EnemyBlink : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    UPROPERTY()
    AEnemy *EnemyActor;

    UPROPERTY(EditDefaultsOnly, Category = "Blackboard")
    FName PointOfInterestKeyName = FName("LastKnownLocation");

    UPROPERTY(EditAnywhere, Category = "Abilities")
    float Strength = 1000.f;

    UFUNCTION()
    void OnRootMotionFinished();
};
