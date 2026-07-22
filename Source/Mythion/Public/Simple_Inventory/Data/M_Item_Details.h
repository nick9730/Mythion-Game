// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "Simple_Invntory/Data/M_ItemTypes.h"
#include "GameplayEffect.h"
#include "Weapons/WeaponBase.h"
#include "M_Item_Details.generated.h"


class AWeaponBase;
struct FGameplayTagContainer;
class AM_Interactive_Item;

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
    EItemType ItemType = EItemType::Inventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Use")
    TSubclassOf<UGameplayEffect> UseEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
    UStaticMesh* StaticMesh = nullptr;

 
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemStats ItemStats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FName Socket;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText DescriptionOfItemForInventorySlot;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TSubclassOf<AM_Interactive_Item> ItemClassToSpawn;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer AllowedClasses;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer ItemTag;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float BuyPrice = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float SellPrice = 0.f;
    



    // Weapon 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (EditCondition = " ItemType  ==  EItemType::Weapon"))
    float Damage = 0.f;



    UPROPERTY(EditAnywhere, Category = "Weapon")
    TSubclassOf<AWeaponBase> WeaponClass;
 



};
