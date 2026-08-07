// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Dogde.generated.h"

/**
 *
 */
class APlayerCharacter;

UCLASS()
class MYTHION_API UGA_Dogde : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    UPROPERTY()
    TObjectPtr<APlayerCharacter> PlayerChar;
};
