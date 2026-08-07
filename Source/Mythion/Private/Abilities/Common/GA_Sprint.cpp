#include "Abilities/Common/GA_Sprint.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystemComponent.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *LocalPlayerChar = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    if (!IsValid(LocalPlayerChar))
        return;

    PlayerChar = LocalPlayerChar;

    UAbilityTask_WaitInputRelease *WaitReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false);

    SetMaxSpeed(600.f);
    ApplyOutgoingEffect(PlayerChar);

    WaitReleaseTask->OnRelease.AddDynamic(this, &UGA_Sprint::OnInputReleased);
    WaitReleaseTask->ReadyForActivation();
}

void UGA_Sprint::OnInputReleased(float TimeHeld)
{
    UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo();

    if (IsValid(ASC))
    {
        FGameplayTagContainer TagsToRemove;
        TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("Data.Energy.GrantedEnergy")));
        ASC->RemoveActiveEffectsWithGrantedTags(TagsToRemove);
    }

    SetMaxSpeed(300.f);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Sprint::SetMaxSpeed(float Speed)
{
    if (IsValid(PlayerChar) && IsValid(PlayerChar->GetCharacterMovement()))
    {
        PlayerChar->GetCharacterMovement()->MaxWalkSpeed = Speed;
    }
}