// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/BlackboardComponent.h"
#include "CoreMinimal.h"
#include "GAS/M_GameplayAbility.h"
#include "GA_ShootProjectile.generated.h"

/**
 *
 */
class AM_BaseProjectile;
class APlayerCharacter;
class AEnemy;
struct FBlackboardKeySelector;
UCLASS()
class MYTHION_API UGA_ShootProjectile : public UM_GameplayAbility
{
    GENERATED_BODY()

  public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData) override;

    UPROPERTY(EditDefaultsOnly, Category = "Abilities | Projectile")
    TSubclassOf<AM_BaseProjectile> Projectile;

    virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Damage")
    float Damage;

    UPROPERTY()
    APlayerCharacter *PlayerChar;

    UPROPERTY()
    AEnemy *EnemyChar;

    UPROPERTY()
    FVector TargetPosition;


    UFUNCTION()
    FVector PredictTargetLocation(AActor *TargetActor, FVector ShooterLocation, float ProjectileSpeed);

    UPROPERTY()
    float CurrentLevel;

    UFUNCTION()
    void HandleProjectileHit(AActor *HitActor, FVector HitLocation);

    UFUNCTION()
    void HandleProjectileExpired();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Distance")
    float FrontDistance = 50.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities | Distance")
    float ProjectileSpeedForPrediction = 2000.f;

    UFUNCTION()
    virtual void SpawnSingleProjectile(FVector Direction);
};
