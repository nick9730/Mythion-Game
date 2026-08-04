// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_AOEAttack.generated.h"

/**
 *
 */

class UAnimMontage;
class APlayerCharacter;
class AGameplayAbilityTargetActor;
UCLASS() class MYTHION_API UGA_AOEAttack : public UM_GameplayAbility
{
    GENERATED_BODY()
  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    UPROPERTY(EditDefaultsOnly, Category = "Abilities | AoE") TSubclassOf<AGameplayAbilityTargetActor> TargetActorClass;

    UFUNCTION()
    void OnTargetDataReady(const FGameplayAbilityTargetDataHandle &TargetData);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Damage")
    float Damage;

    UPROPERTY()
    APlayerCharacter *PlayerChar;

    UPROPERTY()
    float CurrentLevel;

    UFUNCTION()
    void OnMontageAOECompleted();

    UFUNCTION()
    void OnMontageAOEInterrupted();

    UFUNCTION()
    void OnMontageAOECancelled();

    UFUNCTION()
    void PlayMontageCastAOEAttackEnd();

    UFUNCTION()
    void PlayAimMontage();

    UFUNCTION()
    void OnAimMontageInterrupted();

    UFUNCTION()
    void ValidateTargetData();

    UFUNCTION()
    void EndAbilityAOE(float TimeHeld);

    UPROPERTY()
    AGameplayAbilityTargetActor *SpawnedTargetActor;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

    UPROPERTY()
    bool bTargetWasConfirmed = false;

    UPROPERTY()
    FGameplayAbilityTargetDataHandle TargetDataHandle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Montage")
    UAnimMontage *AOEAttackEndAnimMonage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Montage")
    UAnimMontage *AimPoseMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Montage")
    AGameplayAbilityTargetActor *InTargetActor;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Damage")
    float AOERadius;

    UPROPERTY()
    float TotalAOERadius;
};