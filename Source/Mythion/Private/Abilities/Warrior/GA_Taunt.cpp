// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/GA_Taunt.h"
#include "Characters/PlayerCharacter.h"

void UGA_Taunt::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
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

    ApplyOutgoingEffect(PlayerChar);
    PlayMontage();
}
