// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "M_HitReactionByShield.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UM_HitReactionByShield : public UM_GameplayAbility
{
    GENERATED_BODY()

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;
};
