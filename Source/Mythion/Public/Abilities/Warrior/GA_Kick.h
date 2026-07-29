// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_Kick.generated.h"

/**
 *
 */
class APlayerCharacter;
class AEnemy;

UCLASS()
class MYTHION_API UGA_Kick : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

    UFUNCTION()
    void ApplyKickWIthSphereTraceObjects(APlayerCharacter *PlayerChar);

    FVector CalculateStartTrace(APlayerCharacter *Character);

    FVector CalculateEndTrace(APlayerCharacter *Character);

    FVector GetDirectionLaunchVelocity(APlayerCharacter *Character);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | KickDistanse")
    FVector KickDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | KickDistanse")
    FVector KnockBackDistance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | KickRadius")
    float KickRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Damage")
    float Damage;

    UPROPERTY()
    AEnemy *TargetEnemy;

    UFUNCTION()
    void OnDelayFinished();

    UFUNCTION()
    float GetCharacterLevel(APlayerCharacter *Character);

    UPROPERTY()
    float CurrentLevel = 1.0f;

    UPROPERTY()
    APlayerCharacter *InstigatorCharacter;
};
