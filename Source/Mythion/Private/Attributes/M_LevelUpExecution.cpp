#include "Attributes/M_LevelUpExecution.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h"

// Στο top του M_LevelUpExecution.cpp
struct MyLevelUpStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(Level);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Xp);
    DECLARE_ATTRIBUTE_CAPTUREDEF(XpMax);

    MyLevelUpStatics()
    {
        // Κάνουμε capture από τον Target (που είναι ο εαυτός μας)
        DEFINE_ATTRIBUTE_CAPTUREDEF(UM_AttributeSet, Level, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UM_AttributeSet, Xp, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UM_AttributeSet, XpMax, Target, false);
    }
};

static const MyLevelUpStatics& LevelUpStatics()
{
    static MyLevelUpStatics Statics;
    return Statics;
}

UM_LevelUpExecution::UM_LevelUpExecution()
{
    // Εδώ λέμε στο GAS να κάνει capture αυτά τα Attributes πριν τρέξει το Execute
    RelevantAttributesToCapture.Add(LevelUpStatics().LevelDef);
    RelevantAttributesToCapture.Add(LevelUpStatics().XpDef);
    RelevantAttributesToCapture.Add(LevelUpStatics().XpMaxDef);
}

void UM_LevelUpExecution::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

    UAbilitySystemComponent* ASC = ExecutionParams.GetTargetAbilitySystemComponent();


    if (!ASC || !ASC->IsOwnerActorAuthoritative()) return;

    APlayerCharacter* Character = Cast<APlayerCharacter>(ASC->GetAvatarActor());
    if (!Character) return;

    const UM_AttributeSet* AttributeSet = Cast<UM_AttributeSet>(ASC->GetAttributeSet(UM_AttributeSet::StaticClass()));
    if (!AttributeSet) return;

    float CurrentLevel = AttributeSet->GetLevel();
    float CurrentXP = ASC->GetNumericAttributeBase(UM_AttributeSet::GetXpAttribute());

    float XpMax = AttributeSet->GetXpMax();

    float XPGained = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag("Data.XP"), false, 0.f);

    UE_LOG(LogTemp, Warning, TEXT("XP from AttributeSet: %f"), CurrentXP);




    CurrentXP += XPGained;
    while (CurrentXP >= XpMax)
    {
        CurrentXP -= XpMax;
        CurrentLevel += 1.f;
        if (IsValid(Character) && Character->LevelUpStats)
        {
            FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
            EffectContext.AddInstigator(Character, Character);
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Character->LevelUpStats, CurrentLevel, EffectContext);

            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    

        if (IsValid(Character->XpScaleTable))
        {
            XpMax = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(CurrentLevel);
        }
        else
        {
            break;
        }
    }

    // Output Modifiers
    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UM_AttributeSet::GetXpAttribute(), EGameplayModOp::Override, CurrentXP));
    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UM_AttributeSet::GetLevelAttribute(), EGameplayModOp::Override, CurrentLevel));
    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UM_AttributeSet::GetXpMaxAttribute(), EGameplayModOp::Override, XpMax));
}
