// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/GA_MultipleShootProjectiles.h"
#include "Abilities/Mage/GA_ShootProjectile.h"
#include "Characters/PlayerCharacter.h"

void UGA_MultipleShootProjectiles::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo *ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData *TriggerEventData)
{
    UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

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
    NumberOfShots = CurrentLevel + 1;
    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Mage.MultipleShootProjectile"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
}

void UGA_MultipleShootProjectiles::OnGameplayEventReceived(FGameplayEventData Payload)
{
    if (!IsValid(PlayerChar))
        return;
    if (!HasAuthority(&CurrentActivationInfo))
        return;

    FVector BaseDirection = PlayerChar->GetActorForwardVector();

    for (int32 i = 0; i < NumberOfShots; i++)
    {
        float AngleOffset = (i - (NumberOfShots - 1) / 2.0f) * SpreadAngle;
        FVector Direction = BaseDirection.RotateAngleAxis(AngleOffset, FVector::UpVector);
        SpawnSingleProjectile(Direction);
    }
}
