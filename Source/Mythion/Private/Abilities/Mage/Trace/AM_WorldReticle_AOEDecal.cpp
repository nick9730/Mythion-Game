// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/Trace/AM_WorldReticle_AOEDecal.h"
#include "Components/DecalComponent.h"

AAM_WorldReticle_AOEDecal::AAM_WorldReticle_AOEDecal()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange,
                                         FString::Printf(TEXT("AOE: RootComponent valid=%d"), IsValid(RootComponent)));
    USceneComponent *SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
    DecalComponent->SetupAttachment(RootComponent);

    DecalComponent->DecalSize = FVector(50.f, 50.f, 50.f);
    DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    if (DecalMaterial)
    {
        DecalComponent->SetDecalMaterial(DecalMaterial);
    }
}

void AAM_WorldReticle_AOEDecal::BeginPlay()
{
    Super::BeginPlay();
    if (DecalMaterial)
    {
        DecalComponent->SetDecalMaterial(DecalMaterial);
    }
}

void AAM_WorldReticle_AOEDecal::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AAM_WorldReticle_AOEDecal::SetDecalRadius(float Radius)
{
    DecalComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    DecalComponent->DecalSize = FVector(Radius, Radius, Radius);
}