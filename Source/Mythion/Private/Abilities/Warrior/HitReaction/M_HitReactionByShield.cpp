// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Warrior/HitReaction/M_HitReactionByShield.h"
#include "Characters/Enemy.h"

void UM_HitReactionByShield::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo *ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AEnemy *Enemy = Cast<AEnemy>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Enemy))
        return;

    ;

    /*
if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
{
EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
return;
}EventCalledTag
*/
    /*
     */

    if (GEngine)
    {
        FString TagStr = TriggerEventData ? TriggerEventData->EventTag.ToString() : TEXT("NoTriggerData");
        FString InstigatorStr = (TriggerEventData && IsValid(TriggerEventData->Instigator))
                                    ? TriggerEventData->Instigator->GetName()
                                    : TEXT("NoInstigator");

        GEngine->AddOnScreenDebugMessage(
            -1, 5.f, FColor::Magenta,
            FString::Printf(TEXT("HitReactionByShield ACTIVATED on %s | TriggerTag=%s | Instigator=%s"),
                            *Enemy->GetName(), *TagStr, *InstigatorStr));
    }

    PlayMontage();
}
