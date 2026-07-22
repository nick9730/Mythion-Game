// Fill out your copyright notice in the Description page of Project Settings.


#pragma once
#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EItemType : uint8
{
    Weapon,
    Armor,
    Consumable,
    Misc
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common,
    Uncommon,
    Rare,
    Epic
};

UENUM(BlueprintType)
enum class EItemStats : uint8
{
    HealthRefill,
    ManaRefill,
    GainXp,
	ArmorBuff,
	MagicResBuff,
};