// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Enemy_Warrior_Attack.generated.h"

/**
 *
 */

class AEnemy;
struct FGameplayTag;

UCLASS()
class MYTHION_API UGA_Enemy_Warrior_Attack : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload);

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName AttackTargetKeyName = FName("TargetActor");

    UPROPERTY(EditAnywhere, Category = "AI")
    FName BlackBoardKey;

    UPROPERTY()
    TObjectPtr<AEnemy> EnemyTarget;

    UPROPERTY()
    FGameplayTag EventCalledTagForBlocking;

    UFUNCTION()
    void OnBlockedEventReceived(FGameplayEventData Payload);
};
