// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Blink.generated.h"

/**
 *
 */

class APlayerCharacter;

UCLASS()
class MYTHION_API UGA_Blink : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    UGA_Blink();

    UPROPERTY(EditDefaultsOnly, Category = "Blink")
    float DashStrength = 2000.f;

    UPROPERTY()
    float CurrentLevel;

    UFUNCTION()
    void OnRootMotionFinished();

    UPROPERTY()
    APlayerCharacter *PlayerChar;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;
};
