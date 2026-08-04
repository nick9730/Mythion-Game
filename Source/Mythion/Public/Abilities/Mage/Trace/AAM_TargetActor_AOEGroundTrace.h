// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "CoreMinimal.h"

#include "AAM_TargetActor_AOEGroundTrace.generated.h"

UCLASS()
class MYTHION_API AAM_TargetActor_AOEGroundTrace : public AGameplayAbilityTargetActor_GroundTrace
{
    GENERATED_BODY()

  public:
    AAM_TargetActor_AOEGroundTrace();

    virtual void StartTargeting(UGameplayAbility *Ability) override;

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY()
    float DesiredRadius = 200.f;
};
