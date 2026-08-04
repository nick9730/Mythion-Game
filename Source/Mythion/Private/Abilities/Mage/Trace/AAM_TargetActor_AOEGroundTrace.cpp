// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/Trace/AAM_TargetActor_AOEGroundTrace.h"
#include "Abilities/Mage/Trace/AM_WorldReticle_AOEDecal.h"

AAM_TargetActor_AOEGroundTrace::AAM_TargetActor_AOEGroundTrace()
{
    ReticleClass = AAM_WorldReticle_AOEDecal::StaticClass();
    CollisionRadius = 0.f;
    MaxRange = 2000.f;
    bTraceAffectsAimPitch = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    TraceProfile = FCollisionProfileName(TEXT("GroundTargetingTrace"));
}
void AAM_TargetActor_AOEGroundTrace::StartTargeting(UGameplayAbility *Ability)
{
    Super::StartTargeting(Ability);
    if (AGameplayAbilityWorldReticle *LocalReticle = ReticleActor.Get())
    {
        AAM_WorldReticle_AOEDecal *DecalReticle = Cast<AAM_WorldReticle_AOEDecal>(LocalReticle);
        if (IsValid(DecalReticle))
        {
            DecalReticle->SetDecalRadius(DesiredRadius);
        }
    }
}

void AAM_TargetActor_AOEGroundTrace::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}