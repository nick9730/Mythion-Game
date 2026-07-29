// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "M_GameplayAbility.generated.h"

/**
 *
 */
class UAnimMontage;
struct FGameplayTag;

UCLASS()
class MYTHION_API UM_GameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

  public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mythion | Debug")
    bool bDrawDebugs = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythion | UI")
    bool bShouldShowInAbilityBar = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Montage")
    UAnimMontage *PlayableAnimMonage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Montage")
    TSubclassOf<UGameplayEffect> AssignedEffectClass;

    UFUNCTION(BlueprintCallable, Category = "Mythion | Abilities")
    virtual void PlayMontage();

    UFUNCTION(BlueprintCallable, Category = "Event")
    virtual void WaitGameplayEvent(FGameplayTag EventTag);

    UFUNCTION()
    virtual void OnGameplayEventReceived(FGameplayEventData Payload);

    UFUNCTION()
    virtual void ApplyEffectWithMagnitude(AActor *TargetActor, FGameplayTag SetByCallerTag, float Magnitude);

    UFUNCTION()
    virtual void ApplyOutgoingEffect(AActor *TargetActor);

    UFUNCTION(BlueprintCallable, Category = " Abilities")
    virtual void ExecuteCueOnActor(AActor *TargetActor, FGameplayTag CueTag, float RawMagnitude, FVector Location);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | CalledTag")
    FGameplayTag EventCalledTag;

    UFUNCTION()
    virtual void OnMontageCompleted();

    UFUNCTION()
    virtual void OnMontageInterrupted();

    UFUNCTION()
    virtual void OnMontageCancelled();


  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Safety")
float EventTimeoutDuration = 3.0f;

UFUNCTION(BlueprintCallable, Category = "Abilities | Safety")
virtual void StartEventTimeoutSafety();

UFUNCTION()
virtual void OnEventTimeout();

UPROPERTY()
bool bHasEnded = false;
};
