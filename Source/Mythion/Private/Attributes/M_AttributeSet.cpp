// Fill out your copyright notice in the Description page of Project Settings.

#include "Attributes/M_AttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/BaseCharacter.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "Engine/Engine.h"
#include "GameplayEffectExtension.h"
#include "Kismet/GameplayStatics.h"
#include "M_PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Damage.h"
#include "PlayerController/Components/M_RespawnComponent.h"
/*
simple -> webserver  express  or hono
oarem  -> drissle oarem  or micro oarem
validation -> zod
there are some templates for faster solution
*/

UM_AttributeSet::UM_AttributeSet()
{
}

void UM_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Armor, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MagicResistance, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Xp, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, XpMax, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Level, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Coins, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Energy, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);
}

void UM_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
    {

        if (GetMaxEnergy() > 0.f)
        {
            SetMaxEnergy(FMath::Max(GetMaxEnergy(), 1.f));
            SetEnergy(FMath::Clamp(GetEnergy(), 0.f, GetMaxEnergy()));
        }
    }

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {

        if (Data.EvaluatedData.Magnitude < 0.0f)
        {

            if (UAbilitySystemComponent *InstigatorASC =
                    Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent())
            {
                if (APlayerCharacter *InstigatorChar = Cast<APlayerCharacter>(InstigatorASC->GetAvatarActor()))
                {
                    AActor *Owner = GetOwningActor();
                    if (AEnemy *EnemyOwner = Cast<AEnemy>(Owner))
                    {
                        EnemyOwner->LastDamageInstigator = Cast<AM_PlayerController>(InstigatorChar->GetController());
                    }
                }
            }
        }

        if (GetMaxHealth() > 0.f)
        {
            SetMaxHealth(FMath::Max(GetMaxHealth(), 1.f));
            SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
        }

        UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
        if (!IsValid(ASC))
            return;
        bool bStillHasDeadTag = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));

        if (Data.EffectSpec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag("Effects.HitReaction")) &&
            GetHealth() > 0.f && !bStillHasDeadTag)
        {

            FGameplayTagContainer HitReactionTag;
            HitReactionTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Spells.Common...HitReaction")));
            GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(HitReactionTag);
        }

        if (GetHealth() <= 0.f)
        {

            // ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));

            FGameplayTagContainer DeadTag;
            DeadTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));
            GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(DeadTag);

            AActor *Owner = ASC->GetAvatarActor();
            APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(Owner);
            if (IsValid(PlayerChar))
            {
                PlayerChar->DeathLocation = PlayerChar->GetActorLocation(); // <-- ΝΕΟ, εδώ σίγουρα server
            }
            /*
             */

            if (IsValid(PlayerChar))
            {
                AM_PlayerController *PC = Cast<AM_PlayerController>(PlayerChar->GetController());
                if (IsValid(PC))
                    PC->RespawnComponent->Client_ShowRespawnWidget();
            }
        }

        if (Data.EvaluatedData.Magnitude < 0.0f)
        {
            AActor *DamagedCharacter = GetOwningAbilitySystemComponent()->GetAvatarActor();
            AActor *AttackerCharacter = nullptr;

            if (UAbilitySystemComponent *InstigatorASC =
                    Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent())
            {
                AttackerCharacter = InstigatorASC->GetAvatarActor();
            }

            if (!IsValid(AttackerCharacter))
            {
                AttackerCharacter = Data.EffectSpec.GetContext().GetEffectCauser();
            }

            if (IsValid(DamagedCharacter) && IsValid(AttackerCharacter))
            {
                UAISense_Damage::ReportDamageEvent(
                    GetWorld(), DamagedCharacter, AttackerCharacter, FMath::Abs(Data.EvaluatedData.Magnitude),
                    DamagedCharacter->GetActorLocation(), DamagedCharacter->GetActorLocation());
            }
        }
        /*
                if(GetHealth()<= 0.f ){




                    bool bActivated = GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(DeadTag);
                    AActor* Owner = GetOwningAbilitySystemComponent()->GetAvatarActor();
                    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Owner);
                    if (IsValid(PlayerChar))
                    {
                        AM_PlayerController* PC = Cast<AM_PlayerController>(PlayerChar->GetController());
                        if (IsValid(PC))
                            PC->Client_ShowRespawnWidget();
                    }
        */
        /*
        FGameplayTagContainer RespawnTag;
        RespawnTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Alive")));
        GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(RespawnTag);

        */
    }
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        if (GetMaxMana() > 0.f)
        {
            SetMaxMana(FMath::Max(GetMaxMana(), 1.f));
            SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
        }
    }

    if (Data.EvaluatedData.Attribute == GetLevelAttribute())
    {
    }

    if (!bAttributesInitialized)
    {

        bAttributesInitialized = true;

        APlayerCharacter *Character = Cast<APlayerCharacter>(GetOwningActor());
        if (IsValid(Character) && IsValid(Character->XpScaleTable))
        {
            float Levele = FMath::Max(GetLevel(), 1.f);
            XpMaxLimit = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Levele);
            SetXpMax(XpMaxLimit);
        }

        OnAttributesInitialized.Broadcast();
    }
}

void UM_AttributeSet::PostAttributeChange(const FGameplayAttribute &Attribute, float OldValue, float NewValue)
{
    // Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

void UM_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute &Attribute, float &NewValue) const
{

    Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UM_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData &OldValue)
{

    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MaxHealth, OldValue);
}

void UM_AttributeSet::OnRep_Health(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Health, OldValue);
}

void UM_AttributeSet::OnRep_Mana(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Mana, OldValue);
}

void UM_AttributeSet::OnRep_MaxMana(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MaxMana, OldValue);
}

void UM_AttributeSet::OnRep_Armor(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Armor, OldValue);
}

void UM_AttributeSet::OnRep_MagicResistance(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MagicResistance, OldValue);
}

void UM_AttributeSet::OnRep_Xp(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Xp, OldValue);
}

void UM_AttributeSet::OnRep_XpMax(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, XpMax, OldValue);
}

void UM_AttributeSet::OnRep_Level(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Level, OldValue);
}

void UM_AttributeSet::OnRep_Coins(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Coins, OldValue);
}

void UM_AttributeSet::OnRep_Energy(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Energy, OldValue);
}

void UM_AttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData &OldValue)
{
    UAbilitySystemComponent *ASC = GetOwningAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MaxEnergy, OldValue);
}

void UM_AttributeSet::ApplyStatsFromBackend(UAbilitySystemComponent *ASC, TSubclassOf<UGameplayEffect> InitEffect,
                                            float MaxHealth, float MaxMana, float Level, float XP, float XPMax,
                                            float Coins, float Armor, float MagicResist, float Energy, float MaxEnergy,
                                            float Health, float Mana)
{
    if (!IsValid(ASC) || !IsValid(InitEffect))
        return;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(InitEffect, 1, Context);

    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Data.MaxMana"), MaxMana);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Level"),
                                                                  Level);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.XP"),
                                                                  XP);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.XpMax"),
                                                                  XPMax);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins.Initialize"), Coins);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Armor"),
                                                                  Armor);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Data.MagicResist"), MagicResist);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Energy"),
                                                                  Energy);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Data.MaxEnergy"), MaxEnergy);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Health"),
                                                                  Health);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Mana"),
                                                                  Mana);

    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UM_AttributeSet::CalculateClassStats(FGameplayTag PlayerClassTag, UCurveTable *XpScaleTable, int32 Level,
                                          float &OutXpMaxLimit, float &OutMaxMana, float &OutMaxHealth)
{

    if (!IsValid(XpScaleTable))
        return;

    if (PlayerClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Classes.Warrior")))
    {
        OutXpMaxLimit = XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Level);
        OutMaxMana = XpScaleTable->FindCurve(FName("MaxManaWarrior"), "Mana Lookup")->Eval(Level);
        OutMaxHealth = XpScaleTable->FindCurve(FName("MaxHealthWarrior"), "Health Lookup")->Eval(Level);
    }
    else if (PlayerClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Classes.Rogue")))
    {
        OutXpMaxLimit = XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Level);
        OutMaxMana = XpScaleTable->FindCurve(FName("MaxManaMage"), "Mana Lookup")->Eval(Level);
        OutMaxHealth = XpScaleTable->FindCurve(FName("MaxHealthMage"), "Health Lookup")->Eval(Level);
    }
}