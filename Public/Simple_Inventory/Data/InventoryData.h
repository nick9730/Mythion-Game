// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Simple_Invntory/Data/M_ItemTypes.h"
#include "InventoryData.generated.h"


class UM_Item_Details;

USTRUCT(BlueprintType)
struct FItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemType ItemType = EItemType::Misc;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UM_Item_Details* ItemDataAsset = nullptr;

    bool IsValid() const { return !ItemID.IsNone(); }
};
