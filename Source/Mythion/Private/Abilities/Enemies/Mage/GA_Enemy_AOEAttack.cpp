// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Enemies/Mage/GA_Enemy_AOEAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemy.h"
#include "Kismet/GameplayStatics.h"

void UGA_Enemy_AOEAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo *ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AEnemy *Enemy = Cast<AEnemy>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Enemy))
        return;
    EnemyInstigator = Enemy;

    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.Mage.AOE"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
}

void UGA_Enemy_AOEAttack::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    FVector TargetLocation = PositionOfPlayerCharacter(EnemyInstigator);
    ExecuteCueAtLocation(TagForDecal, Radius, TargetLocation);
    ApplyAOEDamageAtLocation(TargetLocation, TargetLocation, Radius, Damage, EnemyInstigator, 1,
                             GamepalyCueTagLocation);
}
