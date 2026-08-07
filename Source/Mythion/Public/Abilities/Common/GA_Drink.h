// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Drink.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UGA_Drink : public UM_GameplayAbility
{
    GENERATED_BODY()
  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;
};
