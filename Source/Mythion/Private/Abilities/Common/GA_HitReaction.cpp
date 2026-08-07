// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Common/GA_HitReaction.h"
#include "Abilities/Common/GA_Dead.h"
#include "Characters/PlayerCharacter.h"

void UGA_HitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo *ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *LocalPlayerChar = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(LocalPlayerChar))
        return;

    PlayerChar = LocalPlayerChar;

    FGameplayTag TagMatches = PlayerChar->PlayerClassTag;

    UCharacterClasses *LoadedClassData = CharacterClassData.LoadSynchronous();
    if (!IsValid(LoadedClassData))
        return;

    for (const FCharacterClassData &Choice : LoadedClassData->CharacterClasses)
    {
        if (Choice.ClassNameTag.MatchesTagExact(TagMatches))
        {
            //   PlayableAnimMonage = Choice.HitReactionMontage;
        }
    }

    PlayMontage();
}