// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M_BaseProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHit, AActor *, HitActor, FVector, HitLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProjectileExpired);

UCLASS()
class MYTHION_API AM_BaseProjectile : public AActor
{
    GENERATED_BODY()

  public:
    AM_BaseProjectile();

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    USphereComponent *CollisionComponent;

    UPROPERTY(BlueprintAssignable)
    FOnProjectileExpired OnProjectileExpired;

    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent *ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    UNiagaraComponent *ProjectileVFX;

    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ProjectileSpeed = 1500.0f;

    UFUNCTION()
    void OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
               const FHitResult &Hit);

    UPROPERTY(BlueprintAssignable)
    FOnProjectileHit OnProjectileHit;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float MaxLifetime = 5.0f;

    UFUNCTION()
    void SetSourceActor(AActor *NewSourceActor)
    {
        SourceActor = NewSourceActor;
    }

    UPROPERTY()
    AActor *SourceActor;

    UFUNCTION()
    void IgnoreActor(AActor *ActorToIgnore);

    UFUNCTION()
    void LaunchInDirection(FVector Direction);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float GravityScale = 0.0f;

  protected:
    bool bHasHit = false;

    virtual void Destroyed() override;
    virtual void BeginPlay() override;

  public:
    virtual void Tick(float DeltaTime) override;
};
