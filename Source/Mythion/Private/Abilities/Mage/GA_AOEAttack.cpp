// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/GA_AOEAttack.h"
#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "Abilities/Mage/Trace/AAM_TargetActor_AOEGroundTrace.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

void UGA_AOEAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo,
                                    const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(PlayerCharacter))
        return;
    /*
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    */

    PlayerChar = PlayerCharacter;
    CurrentLevel = GetCharacterLevel(PlayerChar);
    PlayAimMontage();

    CurrentLevel = GetCharacterLevel(PlayerChar);
    TotalAOERadius = CurrentLevel * AOERadius;

    ValidateTargetData();

    UAbilityTask_WaitInputRelease *WaitReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);
    WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_AOEAttack::EndAbilityAOE);
    WaitReleaseTask->ReadyForActivation();
}

void UGA_AOEAttack::OnTargetDataReady(const FGameplayAbilityTargetDataHandle &TargetData)
{
    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (TargetData.Num() == 0)
        return;

    if (IsValid(SpawnedTargetActor))
    {
        SpawnedTargetActor->Destroy();
        SpawnedTargetActor = nullptr;
    }

    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Mage.AOE"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontageCastAOEAttackEnd();

    TargetDataHandle = TargetData;
}

void UGA_AOEAttack::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    const FGameplayAbilityTargetData *Data = TargetDataHandle.Get(0);
    if (!Data)
    {

        return;
    }

    FVector TargetLocation = Data->GetEndPoint();
    /*
    TArray<AActor *> IgnoreActors;
    IgnoreActors.Add(PlayerChar);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<FHitResult> OutHits;
    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, TargetLocation, TargetLocation, TotalAOERadius,
    ObjectTypes, false, IgnoreActors,
    EDrawDebugTrace::None, OutHits, true);

    float Magnitude = Damage * CurrentLevel;
    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Magical"));

    if (bHit)
    {
        for (const FHitResult &Hit : OutHits)
        {
            AActor *Actor = Hit.GetActor();
            AEnemy *EnemyTarget = Cast<AEnemy>(Actor);
            APlayerCharacter *Character = Cast<APlayerCharacter>(Actor);

            if (IsValid(EnemyTarget))
            {
                ApplyEffectWithMagnitude(EnemyTarget, SetByCallerTag, Magnitude);
            }
            else if (IsValid(Character))
            {
                ApplyEffectWithMagnitude(Character, SetByCallerTag, Magnitude);
            }
        }
    }
    ExecuteCueOnActor(PlayerChar, GamepalyCueTagLocation, TotalAOERadius, TargetLocation);
    */

    ApplyAOEDamageAtLocation(TargetLocation, TargetLocation, TotalAOERadius, Damage, PlayerChar, CurrentLevel,
                             GamepalyCueTagLocation);
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_AOEAttack::PlayMontageCastAOEAttackEnd()
{
    if (!AOEAttackEndAnimMonage)
        return;

    UAbilityTask_PlayMontageAndWait *MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AOEAttackEndAnimMonage, 1.0f, NAME_None, true, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UGA_AOEAttack::OnMontageAOECompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_AOEAttack::OnMontageAOEInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_AOEAttack::OnMontageAOECancelled);

    MontageTask->ReadyForActivation();
}

void UGA_AOEAttack::OnMontageAOECompleted()
{

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_AOEAttack::OnMontageAOEInterrupted()
{

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_AOEAttack::OnMontageAOECancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_AOEAttack::ValidateTargetData()
{

    UWorld *World = GetWorld();
    if (!IsValid(World) || !IsValid(TargetActorClass) || !IsValid(PlayerChar))
        return;

    if (!CurrentActorInfo->IsLocallyControlled())
    {

        UAbilityTask_WaitTargetData *WaitTargetTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
            this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, nullptr);

        WaitTargetTask->ValidData.AddDynamic(this, &UGA_AOEAttack::OnTargetDataReady);
        WaitTargetTask->ReadyForActivation();
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = PlayerChar;

    SpawnedTargetActor = World->SpawnActor<AGameplayAbilityTargetActor>(
        TargetActorClass, PlayerChar->GetActorLocation(), PlayerChar->GetActorRotation(), SpawnParams);

    SpawnedTargetActor->SourceActor = PlayerChar;
    SpawnedTargetActor->StartLocation.SourceActor = PlayerChar;
    SpawnedTargetActor->StartLocation.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;

    AAM_TargetActor_AOEGroundTrace *AOETargetActor = Cast<AAM_TargetActor_AOEGroundTrace>(SpawnedTargetActor);
    if (!IsValid(AOETargetActor))
        return;

    AOETargetActor->DesiredRadius = TotalAOERadius;

    UAbilityTask_WaitTargetData *WaitTargetTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(
        this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, SpawnedTargetActor);

    WaitTargetTask->ValidData.AddDynamic(this, &UGA_AOEAttack::OnTargetDataReady);
    WaitTargetTask->ReadyForActivation();
}

void UGA_AOEAttack::EndAbilityAOE(float TimeHeld)
{
    if (IsValid(SpawnedTargetActor))
    {
        SpawnedTargetActor->Destroy();
        SpawnedTargetActor = nullptr;
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_AOEAttack::PlayAimMontage()
{
    if (!AimPoseMontage)
        return;

    UAbilityTask_PlayMontageAndWait *AimMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, AimPoseMontage, 1.0f, NAME_None, true, 1.0f);

    AimMontageTask->OnInterrupted.AddDynamic(this, &UGA_AOEAttack::OnAimMontageInterrupted);

    AimMontageTask->ReadyForActivation();
}

void UGA_AOEAttack::OnAimMontageInterrupted()
{
    /*
    if (IsValid(SpawnedTargetActor))
    {
        SpawnedTargetActor->Destroy();
        SpawnedTargetActor = nullptr;
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    */
}
