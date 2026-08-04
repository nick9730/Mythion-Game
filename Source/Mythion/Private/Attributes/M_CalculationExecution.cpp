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
    {

        return;
    }

    AActor *TargetActor = ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor();
    AActor *SourceActor = ExecutionParams.GetSourceAbilitySystemComponent()->GetAvatarActor();

    FVector TargetForward = TargetActor->GetActorForwardVector();
    FVector ToAttacker = (SourceActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();

    FGameplayCueParameters CueParams;
    CueParams.Instigator = SourceActor;
    CueParams.EffectContext = ExecutionParams.GetOwningSpec().GetEffectContext();

    FGameplayEventData EventData;
    EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Blocked"));
    EventData.Instigator = TargetActor;
    EventData.Target = SourceActor;

    FGameplayEventData EventForPlayerHit;
    EventForPlayerHit.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Player.Hit"));
    EventForPlayerHit.Instigator = SourceActor;
    EventForPlayerHit.Target = TargetActor;

    float DotProduct = FVector::DotProduct(TargetForward, ToAttacker);

    const float BlockAngleThreshold = 0.866f;

    bool bIsFrontalHit = DotProduct > BlockAngleThreshold;

    const FGameplayEffectSpec &Spec = ExecutionParams.GetOwningSpec();

    float Damage = 0.f;

    FGameplayTagContainer AssetTags;
    Spec.GetAllAssetTags(AssetTags);

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Physical")))
        Damage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.Physical"), false, 0.f);

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Magical")))
        Damage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Damage.Magical"), false, 0.f);

    float Armor = TargetASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());
    float MagicResistance = TargetASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());

    float FinalDamage = Damage;

    FGameplayTag IsBlocking = FGameplayTag::RequestGameplayTag(FName("Status.IsBlocking"));

    if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Physical")))
    {

        if (TargetASC->HasMatchingGameplayTag(IsBlocking) && bIsFrontalHit)
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
    else if (AssetTags.HasTag(FGameplayTag::RequestGameplayTag("Data.Damage.Magical")))
        if (TargetASC->HasMatchingGameplayTag(IsBlocking) && bIsFrontalHit)
        {
            FinalDamage *= (80.f / (80.f + MagicResistance));
        }
    FinalDamage *= (100.f / (100.f + MagicResistance));

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(UM_AttributeSet::GetHealthAttribute(), EGameplayModOp::AddBase, -FinalDamage));
}
