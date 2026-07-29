// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/GA_BasicAttack.h"
#include "GAS/M_GameplayAbility.h"

void UGA_BasicAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo *ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.MeleeHit"));

    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
}

void UGA_BasicAttack::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    AActor *Target = const_cast<AActor *>(Payload.Target.Get());
    float Magnitude = Payload.EventMagnitude;
    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Physical"));
    ApplyEffectWithMagnitude(Target, SetByCallerTag, Magnitude);
}
