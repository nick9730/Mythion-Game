// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Enemies/Warrior/GA_Enemy_Warrior_Attack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemy.h"

void UGA_Enemy_Warrior_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo *ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AEnemy *Enemy = Cast<AEnemy>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Enemy))
        return;

    EnemyTarget = Enemy;

    /*
if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
{
EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
return;
}EventCalledTag
*/

    EventCalledTagForBlocking = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Blocked"));

    UAbilityTask_WaitGameplayEvent *WaitBlockedTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventCalledTagForBlocking, nullptr, false, true);
    WaitBlockedTask->EventReceived.AddDynamic(this, &UGA_Enemy_Warrior_Attack::OnBlockedEventReceived);
    WaitBlockedTask->ReadyForActivation();

    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.MeleeHit"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
}

void UGA_Enemy_Warrior_Attack::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    AActor *Target = const_cast<AActor *>(Payload.Target.Get());
    float Magnitude = Payload.EventMagnitude;
    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Physical"));

    ApplyEffectWithMagnitude(Target, SetByCallerTag, Magnitude);
}

void UGA_Enemy_Warrior_Attack::OnBlockedEventReceived(FGameplayEventData Payload)
{

    UAbilitySystemComponent *ASC = EnemyTarget->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        ASC->CurrentMontageStop(0.1f);
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

    if (IsValid(ASC))
    {
        FGameplayTagContainer HitReactionTag;
        HitReactionTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Enemy.Melee.Shield")));
        ASC->TryActivateAbilitiesByTag(HitReactionTag);
    }
}