// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Common/GA_Drink.h"

void UGA_Drink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    PlayMontage();
}
