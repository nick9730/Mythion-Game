// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Enemy_AOEAttack.generated.h"

/**
 *
 */
class AEnemy;

UCLASS()
class MYTHION_API UGA_Enemy_AOEAttack : public UM_GameplayAbility
{
    GENERATED_BODY()
  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

 

    UPROPERTY()
    AEnemy *EnemyInstigator;

      UPROPERTY(EditDefaultsOnly,Category="Abilities")
      float Damage=200.f;

       UPROPERTY(EditDefaultsOnly,Category="Abilities")
      float Radius=1000.f;
};
