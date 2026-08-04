// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/GameplayAbilityWorldReticle.h"
#include "Components/DecalComponent.h"
#include "CoreMinimal.h"

#include "AM_WorldReticle_AOEDecal.generated.h"
/**
 *
 */
UCLASS()
class MYTHION_API AAM_WorldReticle_AOEDecal : public AGameplayAbilityWorldReticle
{
    GENERATED_BODY()

  public:
    AAM_WorldReticle_AOEDecal();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION()
    void SetDecalRadius(float Radius);

  protected:
    UPROPERTY(VisibleAnywhere, Category = "Decal")
    UDecalComponent *DecalComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Decal")
    UMaterialInterface *DecalMaterial;
};
