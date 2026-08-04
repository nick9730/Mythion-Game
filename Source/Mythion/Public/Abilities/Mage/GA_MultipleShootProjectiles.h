// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Mage/GA_ShootProjectile.h"
#include "CoreMinimal.h"
#include "GA_MultipleShootProjectiles.generated.h"

/**
 *
 */
UCLASS()
class MYTHION_API UGA_MultipleShootProjectiles : public UGA_ShootProjectile
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

    UPROPERTY()
    int32 NumberOfShots = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | MultiShoot")
    float SpreadAngle = 10.0f;
};
