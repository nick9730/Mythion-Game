// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "Simple_Invntory/Data/M_ItemTypes.h"
#include "GameplayEffect.h"
#include "M_Item_Details.generated.h"




UCLASS(BlueprintType)
class MYTHION_API UM_Item_Details : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    EItemType ItemType = EItemType::Misc;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use")
    TSubclassOf<UGameplayEffect> UseEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMesh* StaticMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    USkeletalMesh* SkeletalMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemStats ItemStats;

    // Weapon 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (EditCondition = " ItemType  ==  EItemType::Weapon"))
    float Damage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (EditCondition = " ItemType  ==  EItemType::Weapon"))
    float AttackSpeed = 0.f;

    // Armor 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor Stats", meta = (EditCondition = " ItemType  ==  EItemType::Armor"))
    float ArmorValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Armor Stats", meta = (EditCondition = " ItemType  ==  EItemType::Armor"))
    float MagicResistance = 0.f;

    // Consumable 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable Stats", meta = (EditCondition = " ItemType  ==  EItemType::Consumable"))
    float HealthRestore = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable Stats", meta = (EditCondition = "ItemType  ==  EItemType::Consumable"))
    float ManaRestore = 0.f;
};
