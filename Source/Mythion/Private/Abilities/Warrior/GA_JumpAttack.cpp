// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/GA_JumpAttack.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"

void UGA_JumpAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo *ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(PlayerCharacter))
        return;

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    PlayerChar = PlayerCharacter;
    CurrentLevel = GetCharacterLevel(PlayerChar);
    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Warrior.JumpAttack"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
    StartEventTimeoutSafety();
}

void UGA_JumpAttack::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.WarriorAOE"));

    float RawMagnitude = AOERadius * CurrentLevel;
    FVector Location = PlayerChar->GetActorLocation();

    ExecuteCueOnActor(PlayerChar, CueTag, RawMagnitude, Location);
    ApplyJumpAttackWithMultipleSphereTraceObjects(PlayerChar);
}

void UGA_JumpAttack::ApplyJumpAttackWithMultipleSphereTraceObjects(APlayerCharacter *Character)
{

    if (!IsValid(PlayerChar))
        return;

    FVector ApplyLocation = PlayerChar->GetActorLocation();

    TArray<AActor *> IgnoreActors;
    IgnoreActors.Add(PlayerChar);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<FHitResult> OutHits;
    bool bHit =
        UKismetSystemLibrary::SphereTraceMultiForObjects(this, ApplyLocation, ApplyLocation, AOERadius, ObjectTypes,
                                                         false, IgnoreActors, EDrawDebugTrace::None, OutHits, true);

    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Physical"));
    float Magnitude = Damage * CurrentLevel;

    if (bHit)
    {
        for (const FHitResult &Hit : OutHits)
        {
            AEnemy *EnemyTarget = Cast<AEnemy>(Hit.GetActor());
            if (IsValid(EnemyTarget))
            {
                ApplyEffectWithMagnitude(EnemyTarget, SetByCallerTag, Magnitude);
            }
        }
    }
}
