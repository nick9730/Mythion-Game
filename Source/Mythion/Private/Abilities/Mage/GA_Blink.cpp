// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Mage/GA_Blink.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

UGA_Blink::UGA_Blink()
{
	
}

float UGA_Blink::GetMaxBlinkDistance() const
{
    float FinalMaxDistance = MaxBlinkDistance;

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        const float Level = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
		FinalMaxDistance += (Level-1) * BlinkDistancePerLevel;
    }
    return FinalMaxDistance;
}

void UGA_Blink::OnDashFinished()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
        GetCurrentActivationInfo(), true, false);
}


void UGA_Blink::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector Direction = Character->GetActorForwardVector();
    Direction.Z = 0.f;
    Direction.Normalize();

    UAbilityTask_ApplyRootMotionConstantForce* Task =
        UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
            this,
            FName("Dash"),
            Direction,
            DashStrength,
            0.2f,
            false,
            nullptr,
            ERootMotionFinishVelocityMode::ClampVelocity,
            FVector::ZeroVector,
            0.f,
            false
        );

    Task->OnFinish.AddDynamic(this, &UGA_Blink::OnDashFinished);
    Task->ReadyForActivation();
}



FVector UGA_Blink::ValidateBlinkTarget(const FVector& RawTarget, const AActor* Avatar) const
{
    if (RawTarget.IsZero()) return Avatar->GetActorLocation();

    FVector Origin = Avatar->GetActorLocation();
    FVector Direction = (RawTarget - Origin);
    float Distance = Direction.Size();
    Direction.Normalize();

    float ClampedDistance = FMath::Clamp(Distance, MinBlinkDistance, GetMaxBlinkDistance());
    FVector FinalTarget = Origin + Direction * ClampedDistance;

    FHitResult SweepHit;
    FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(35.f, 90.f);

    GetWorld()->SweepSingleByChannel(SweepHit, Origin, FinalTarget,
        FQuat::Identity, ECC_Pawn, CapsuleShape);

    if (SweepHit.bBlockingHit)
        return SweepHit.Location;

    return FinalTarget;
}
