// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Common/GA_Dead.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/PlayerCharacter.h"
#include "DataAsset/CharacterClasses.h"

void UGA_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
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
            PlayableAnimMonage = Choice.DeathMontage;
        }
    }
    UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo();

    if (IsValid(ASC))
    {
        FGameplayTagContainer TagsToRemove;
        TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Alive")));
        ASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
    }
    ApplyOutgoingEffect(PlayerChar);
    PlayMontage();
}
