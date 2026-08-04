#include "Abilities/Enemies/Mage/GA_EnemyBlink.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemy.h"

void UGA_EnemyBlink::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo *ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    AEnemy *Enemy = Cast<AEnemy>(ActorInfo->AvatarActor.Get());
    if (!IsValid(Enemy))
        return;

    AM_Enemy_Controller *EnemyController = Cast<AM_Enemy_Controller>(Enemy->GetController());
    if (!IsValid(EnemyController))
        return;

    UBlackboardComponent *BlackboardComp = EnemyController->GetBlackboardComponent();
    if (!IsValid(BlackboardComp))
        return;

    EnemyActor = Enemy;

    FVector PointOfInterestVector = BlackboardComp->GetValueAsVector(PointOfInterestKeyName);
    FVector ForceDirection = PointOfInterestVector - Enemy->GetActorLocation();
    FVector FinalForceDirection = ForceDirection.GetSafeNormal();

    UAbilitySystemComponent *ASC = Enemy->GetAbilitySystemComponent();
    FGameplayTag BlinkCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.Blink"));

    if (IsValid(ASC))
    {
        FGameplayCueParameters CueParams;
        ASC->AddGameplayCue(BlinkCueTag, CueParams);
    }

    UAbilityTask_ApplyRootMotionConstantForce *RootMotionTask =
        UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
            this, NAME_None, FinalForceDirection, Strength, 0.25f, true, nullptr,
            ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, FVector::ZeroVector, 0.f, true);

    RootMotionTask->OnFinish.AddDynamic(this, &UGA_EnemyBlink::OnRootMotionFinished);
    RootMotionTask->ReadyForActivation();
}

void UGA_EnemyBlink::OnRootMotionFinished()
{
    UAbilitySystemComponent *ASC = EnemyActor->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        FGameplayTag BlinkCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.Blink"));
        ASC->RemoveGameplayCue(BlinkCueTag);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}