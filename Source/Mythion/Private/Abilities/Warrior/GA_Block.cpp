// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/GA_Block.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystemComponent.h"
#include "Characters/PlayerCharacter.h"

void UGA_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(PlayerChar))
        return;

    UAbilityTask_WaitInputRelease *WaitReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);

    ApplyOutgoingEffect(PlayerChar);
    WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_Block::OnInputReleased);
    WaitReleaseTask->ReadyForActivation();
}

void UGA_Block::OnInputReleased(float TimeHeld)
{
    UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo();

    if (IsValid(ASC))
    {
        FGameplayTagContainer TagsToRemove;
        TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.IsBlocking")));
        ASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
    }

    CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}