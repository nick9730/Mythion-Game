// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/GA_Kick.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UGA_Kick::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo,
                               const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(PlayerChar))
        return;

    InstigatorCharacter = PlayerChar;
    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Warrior.Kick"));
    CurrentLevel = GetCharacterLevel(PlayerChar);
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
    StartEventTimeoutSafety();
}

void UGA_Kick::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    float Magnitude = CurrentLevel * Damage;
    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Physical"));
    ApplyEffectWithMagnitude(TargetEnemy, SetByCallerTag, Magnitude);
    ApplyKickWIthSphereTraceObjects(InstigatorCharacter);
}

void UGA_Kick::ApplyKickWIthSphereTraceObjects(APlayerCharacter *PlayerChar)
{
    if (!IsValid(PlayerChar))
        return;

    TArray<AActor *> IgnoreActors;
    IgnoreActors.Add(PlayerChar);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    FHitResult OutHit;
    bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
        this, CalculateStartTrace(PlayerChar), CalculateEndTrace(PlayerChar), KickRadius, ObjectTypes, false,
        IgnoreActors, EDrawDebugTrace::None, OutHit, true);

    if (bHit)
    {
        AEnemy *EnemyTarget = Cast<AEnemy>(OutHit.GetActor());
        if (!IsValid(EnemyTarget))
            return;
        if (!HasAuthority(&CurrentActivationInfo))
            return;

        TargetEnemy = EnemyTarget;
        TargetEnemy->LaunchCharacter(GetDirectionLaunchVelocity(PlayerChar), true, true);
    }
    /*
    else
    {
        UAbilityTask_WaitDelay *DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.5f);
        DelayTask->OnFinish.AddDynamic(this, &UGA_Kick::OnDelayFinished);
        DelayTask->ReadyForActivation();
    }
        */
}

FVector UGA_Kick::CalculateStartTrace(APlayerCharacter *Character)
{
    if (!IsValid(Character))
        return FVector::ZeroVector;

    USkeletalMeshComponent *CharSkeleton = Character->GetMesh();
    FVector SocketLocation = CharSkeleton->GetSocketLocation(FName("KickSocket"));

    return SocketLocation;
}

FVector UGA_Kick::CalculateEndTrace(APlayerCharacter *Character)
{
    if (!IsValid(Character))
        return FVector::ZeroVector;

    FVector PlayerLocation = Character->GetActorLocation();
    FVector PlayerForwardLocation = Character->GetActorForwardVector();

    FVector DistanceToFront = PlayerForwardLocation * KickDistance;

    FVector EndDistanceToFront = DistanceToFront + PlayerLocation;

    return EndDistanceToFront;
}

FVector UGA_Kick::GetDirectionLaunchVelocity(APlayerCharacter *Character)
{
    if (!IsValid(Character))
        return FVector::ZeroVector;

    FVector PlayerForwardLocation = Character->GetActorForwardVector();

    FVector DirectionVelocity = PlayerForwardLocation * KnockBackDistance;
    DirectionVelocity.Z = 150.0f;
    return DirectionVelocity;
}

void UGA_Kick ::OnDelayFinished()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

float UGA_Kick::GetCharacterLevel(APlayerCharacter *Character)
{
    if (!IsValid(Character))
        return 1.0f;

    UAbilitySystemComponent *ASC = Character->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return 1.0f;
    const UM_AttributeSet *AttributeSet = ASC->GetSet<UM_AttributeSet>();
    float PlayerLevel = 1.0f;
    if (IsValid(AttributeSet))
    {
        PlayerLevel = AttributeSet->GetLevel();
    }

    return PlayerLevel;
}