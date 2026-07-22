// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/Widgets/M_HoverInfosItems.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Simple_Inventory/Data/M_Item_Details.h"

void UM_HoverInfosItems::InitHoverInfosItems(const FItemData& Item)
{
    if (!Item.IsValid() || !IsValid(Item.ItemDataAsset)) return;

    if (ItemNameText)
        ItemNameText->SetText(Item.ItemDataAsset->ItemName);

    if (ItemDescriptionText)
        ItemDescriptionText->SetText(Item.ItemDataAsset->DescriptionOfItemForInventorySlot);

    if (ItemIcon && Item.ItemDataAsset->Icon)
        ItemIcon->SetBrushFromTexture(Item.ItemDataAsset->Icon);
}
