// Fill out your copyright notice in the Description page of Project Settings.

#include "Attributes/M_CalculationExecution.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"

void UM_CalculationExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters &ExecutionParams,
                                                     FGameplayEffectCustomExecutionOutput &OutExecutionOutput) const
{
    UAbilitySystemComponent *TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
    UAbilitySystemComponent *SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    if (!TargetASC || !SourceASC)
        return;

    FGameplayTag TeamTag = FGameplayTag::RequestGameplayTag(FName("Team.Enemy"));

    if (SourceASC->HasMatchingGameplayTag(TeamTag) && TargetASC->HasMatchingGameplayTag(TeamTag))
        return;

    AActor *TargetActor = ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor();
    AActor *SourceActor = ExecutionParams.GetSourceAbilitySystemComponent()->GetAvatarActor();

    FGameplayCueParameters CueParams;
    CueParams.Instigator = SourceActor;
    CueParams.EffectContext = ExecutionParams.GetOwningSpec().GetEffectContext();

    FGameplayEventData EventData;
    BuildGameplayEvent(EventData, FName("Event.Attack.Blocked"), TargetActor, SourceActor);

    FGameplayEventData EventForPlayerHit;
    BuildGameplayEvent(EventForPlayerHit, FName("Event.Player.Hit"), TargetActor, SourceActor);

    float Damage = 0.f;

    bool bIsFrontalHit = false;
    FindAndSetIsFrontalHit(TargetActor, SourceActor, bIsFrontalHit);

    const FGameplayEffectSpec &Spec = ExecutionParams.GetOwningSpec();
    FGameplayTagContainer AssetTags;
    Spec.GetAllAssetTags(AssetTags);

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Physical")))
        Damage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.Physical"), false, 0.f);

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Magical")))
        Damage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.Magical"), false, 0.f);

    float FinalDamage = Damage;

    FGameplayTag IsBlockingTag = FGameplayTag::RequestGameplayTag(FName("Status.IsBlocking"));

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Physical")))
    {
        ApplyPhysicalDamage(FinalDamage, TargetASC, SourceActor, TargetActor, CueParams, EventData, EventForPlayerHit,
                            ExecutionParams, bIsFrontalHit, IsBlockingTag);
    }
    else if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Magical")))
    {
        ApplyMagicalDamage(FinalDamage, TargetASC, bIsFrontalHit, IsBlockingTag);
    }

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(UM_AttributeSet::GetHealthAttribute(), EGameplayModOp::AddBase, -FinalDamage));
}

void UM_CalculationExecution::ApplyPhysicalDamage(float &FinalDamage, UAbilitySystemComponent *TargetASC,
                                                  AActor *SourceActor, AActor *TargetActor,
                                                  FGameplayCueParameters CueParams, FGameplayEventData EventData,
                                                  FGameplayEventData EventForPlayerHit,
                                                  const FGameplayEffectCustomExecutionParameters &ExecutionParams,
                                                  bool bIsFrontalHit, FGameplayTag IsBlockingTag) const
{
    float Armor = TargetASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());

    if (TargetASC->HasMatchingGameplayTag(IsBlockingTag) && bIsFrontalHit)
    {
        ExecutionParams.GetTargetAbilitySystemComponent()->ExecuteGameplayCue(
            FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.Block")), CueParams);

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(SourceActor, EventData.EventTag, EventData);
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventForPlayerHit.EventTag,
                                                                 EventForPlayerHit);
        return;
    }

    ExecutionParams.GetTargetAbilitySystemComponent()->ExecuteGameplayCue(
        FGameplayTag::RequestGameplayTag(FName("GameplayCue.Abilities.FleshHit")), CueParams);

    FinalDamage *= (100.f / (100.f + Armor));
}

void UM_CalculationExecution::ApplyMagicalDamage(float &FinalDamage, UAbilitySystemComponent *TargetASC,
                                                 bool bIsFrontalHit, FGameplayTag IsBlockingTag) const
{
    float MagicResistance = TargetASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());

    if (TargetASC->HasMatchingGameplayTag(IsBlockingTag) && bIsFrontalHit)
    {
        FinalDamage *= (80.f / (80.f + MagicResistance));
    }
    FinalDamage *= (100.f / (100.f + MagicResistance));
}

void UM_CalculationExecution::BuildGameplayEvent(FGameplayEventData &EventData, FName EventTag, AActor *Instigator,
                                                 AActor *Target) const
{
    EventData.EventTag = FGameplayTag::RequestGameplayTag(EventTag);
    EventData.Instigator = Instigator;
    EventData.Target = Target;
}

void UM_CalculationExecution::FindAndSetIsFrontalHit(AActor *TargetActor, AActor *SourceActor,
                                                     bool &bOutIsFrontalHit) const
{
    FVector TargetForward = TargetActor->GetActorForwardVector();
    FVector ToAttacker = (SourceActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
    float DotProduct = FVector::DotProduct(TargetForward, ToAttacker);
    const float BlockAngleThreshold = 0.866f;
    bOutIsFrontalHit = DotProduct > BlockAngleThreshold;
}

/*
 EventForPlayerHit.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Player.Hit"));
 EventForPlayerHit.Instigator = SourceActor;
 EventForPlayerHit.Target = TargetActor;

 */

/*
EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Blocked"));
EventData.Instigator = TargetActor;
EventData.Target = SourceActor;
*/
