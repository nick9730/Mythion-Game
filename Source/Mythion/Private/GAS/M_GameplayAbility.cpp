

#include "GAS/M_GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UM_GameplayAbility::PlayMontage()
{
    if (!PlayableAnimMonage)
        return;

    UAbilityTask_PlayMontageAndWait *MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, PlayableAnimMonage, 1.0f, NAME_None, true, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UM_GameplayAbility::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UM_GameplayAbility::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UM_GameplayAbility::OnMontageCancelled);

    MontageTask->ReadyForActivation();
}

void UM_GameplayAbility::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UM_GameplayAbility::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UM_GameplayAbility::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UM_GameplayAbility::WaitGameplayEvent(FGameplayTag EventTag)
{

    UAbilityTask_WaitGameplayEvent *WaitEventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
    WaitEventTask->EventReceived.AddDynamic(this, &UM_GameplayAbility::OnGameplayEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UM_GameplayAbility::OnGameplayEventReceived(FGameplayEventData Payload)
{
}

void UM_GameplayAbility::ApplyEffectWithMagnitude(AActor *TargetActor, FGameplayTag SetByCallerTag, float Magnitude)

{

    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (!IsValid(TargetActor) || !AssignedEffectClass)
        return;

    UAbilitySystemComponent *SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
        return;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return;

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle =
        SourceASC->MakeOutgoingSpec(AssignedEffectClass, GetAbilityLevel(), ContextHandle);

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(SetByCallerTag, Magnitude);
        TargetASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}

void UM_GameplayAbility::ApplyOutgoingEffect(AActor *TargetActor)
{
    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (!AssignedEffectClass)
        return;

    UAbilitySystemComponent *SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
        return;

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle =
        SourceASC->MakeOutgoingSpec(AssignedEffectClass, GetAbilityLevel(), ContextHandle);

    if (SpecHandle.IsValid())
    {
        SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UM_GameplayAbility::ExecuteCueOnActor(AActor *TargetActor, FGameplayTag CueTag, float RawMagnitude,
                                           FVector Location)
{
    if (!IsValid(TargetActor))
        return;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return;

    FGameplayCueParameters CueParams;
    CueParams.Location = Location;
    CueParams.RawMagnitude = RawMagnitude;
    CueParams.SourceObject = this;

    TargetASC->ExecuteGameplayCue(CueTag, CueParams);
}

void UM_GameplayAbility::StartEventTimeoutSafety()
{
    UAbilityTask_WaitDelay *SafetyTimeout = UAbilityTask_WaitDelay::WaitDelay(this, EventTimeoutDuration);
    SafetyTimeout->OnFinish.AddDynamic(this, &UM_GameplayAbility::OnEventTimeout);
    SafetyTimeout->ReadyForActivation();
}

void UM_GameplayAbility::OnEventTimeout()
{
    if (!bHasEnded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability %s timed out waiting for gameplay event - possible network issue"),
               *GetName());
        bHasEnded = true;
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}