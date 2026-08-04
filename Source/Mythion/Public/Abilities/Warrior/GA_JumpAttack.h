// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_JumpAttack.generated.h"

/**
 *
 */
class APlayerCharacter;

UCLASS()
class MYTHION_API UGA_JumpAttack : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

    UPROPERTY()
    float CurrentLevel;

    UFUNCTION()
    void ApplyJumpAttackWithMultipleSphereTraceObjects(APlayerCharacter *Character);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | AOERadius")
    float AOERadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Damage")
    float Damage;

    UPROPERTY()
    APlayerCharacter *PlayerChar;
};
