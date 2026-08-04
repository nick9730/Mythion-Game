// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/GA_Blink.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

UGA_Blink::UGA_Blink()
{
}

void UGA_Blink::OnRootMotionFinished()
{
    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        FGameplayTag BlinkCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.Blink"));
        ASC->RemoveGameplayCue(BlinkCueTag);
    }
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void UGA_Blink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    APlayerCharacter *Character = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!Character)
    {
        return;
    }

    PlayerChar = Character;
    CurrentLevel = GetCharacterLevel(PlayerChar);

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    FGameplayTag BlinkCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.Blink"));

    float FinalStrength = DashStrength * CurrentLevel;

    if (IsValid(ASC))
    {
        FGameplayCueParameters CueParams;
        ASC->AddGameplayCue(BlinkCueTag, CueParams);
    }

    FVector InputDirection = PlayerChar->GetLastMovementInputVector();
    if (InputDirection.IsNearlyZero())
    {
        InputDirection = PlayerChar->GetActorForwardVector();
    }

    UAbilityTask_ApplyRootMotionConstantForce *RootMotionTask =
        UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
            this, NAME_None, InputDirection, FinalStrength, 0.25f, true, nullptr,
            ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0.f, true);

    RootMotionTask->OnFinish.AddDynamic(this, &UGA_Blink::OnRootMotionFinished);
    RootMotionTask->ReadyForActivation();
}
