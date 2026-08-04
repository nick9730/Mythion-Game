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
#include "M_PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Damage.h"
#include <Kismet/GameplayStatics.h>
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
                    PC->Client_ShowRespawnWidget();
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