

// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CoreMinimal.h"
#include "M_AttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)                                                                   \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)                                                         \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)                                                                       \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)                                                                       \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributesInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, float, NewLevel);

UCLASS()
class MYTHION_API UM_AttributeSet : public UAttributeSet
{
    GENERATED_BODY()

  public:
    UM_AttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data) override;
    virtual void PostAttributeChange(const FGameplayAttribute &Attribute, float OldValue, float NewValue) override;
    virtual void PreAttributeBaseChange(const FGameplayAttribute &Attribute, float &NewValue) const override;

    UPROPERTY(BlueprintReadOnly)
    mutable float XpMaxLimit;

    UPROPERTY(BlueprintReadOnly)
    mutable float CurrentLevel;

    // Apply Stats
    static void ApplyStatsFromBackend(UAbilitySystemComponent *ASC, TSubclassOf<UGameplayEffect> InitEffect,
                                      float MaxHealth, float MaxMana, float Level, float XP, float XPMax, float Coins,
                                      float Armor, float MagicResist, float Energy, float MaxEnergy, float Health,
                                      float Mana);
    // Calculate Class Stats
    static void CalculateClassStats(FGameplayTag PlayerClassTag, class UCurveTable *XpScaleTable, int32 Level,
                                    float &OutXpMaxLimit, float &OutMaxMana, float &OutMaxHealth);

    UPROPERTY(BlueprintAssignable)
    FOnAttributesInitialized OnAttributesInitialized;

    // Health
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Health);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, MaxHealth);

    // Mana
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana)
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Mana);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana)
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, MaxMana);

    // Armor
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor)
    FGameplayAttributeData Armor;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Armor);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MagicResistance)
    FGameplayAttributeData MagicResistance;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, MagicResistance);

    // Progression
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Xp)
    FGameplayAttributeData Xp;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Xp);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_XpMax)
    FGameplayAttributeData XpMax;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, XpMax);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level)
    FGameplayAttributeData Level;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Level);

    // Currency
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Coins)
    FGameplayAttributeData Coins;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Coins);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy)
    FGameplayAttributeData Energy;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, Energy);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy)
    FGameplayAttributeData MaxEnergy;
    ATTRIBUTE_ACCESSORS(UM_AttributeSet, MaxEnergy);

    FActiveGameplayEffectHandle XpMaxEffectHandle;

    UPROPERTY(BlueprintAssignable)
    mutable FOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintReadOnly)
    bool bIsLevelingUp = false;

  private:
    UFUNCTION() void OnRep_Health(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Mana(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_MaxMana(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Armor(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_MagicResistance(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Xp(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_XpMax(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Level(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Coins(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_Energy(const FGameplayAttributeData &OldValue);
    UFUNCTION() void OnRep_MaxEnergy(const FGameplayAttributeData &OldValue);

    bool bAttributesInitialized = false;
};
