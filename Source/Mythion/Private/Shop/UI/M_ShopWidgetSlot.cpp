// Fill out your copyright notice in the Description page of Project Settings.

#include "Shop/UI/M_ShopWidgetSlot.h"
#include "Characters/PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_ShopComponent.h"

void UM_ShopWidgetSlot::InitSlot(UM_Item_Details *Item)
{

    if (!IsValid(Item))
        return;

    CurrentItem = Item;

    if (ItemImage && Item->Icon)
        ItemImage->SetBrushFromTexture(Item->Icon);

    if (ItemName)
        ItemName->SetText(Item->ItemName);

    if (ItemPrice)
        ItemPrice->SetText(FText::FromString(FString::FromInt(Item->BuyPrice)));

    if (ItemDescription)
        ItemDescription->SetText(Item->ItemDescription);

    if (ItemQuantity)
        ItemQuantity->SetText(FText::FromString(TEXT("1")));
}

void UM_ShopWidgetSlot::OnBuyButtonClicked()
{
    AM_PlayerController *PC = Cast<AM_PlayerController>(GetOwningPlayer());

    FItemData NewItem;
    NewItem.ItemID = CurrentItem->ItemID;
    NewItem.ItemName = CurrentItem->ItemName;
    NewItem.ItemType = CurrentItem->ItemType;
    NewItem.MaxStackSize = CurrentItem->MaxStackSize;
    NewItem.Icon = CurrentItem->Icon;
    NewItem.Quantity = 1;
    NewItem.ItemDataAsset = CurrentItem;
    NewItem.Socket = CurrentItem->Socket;

    if (IsValid(PC) && IsValid(CurrentItem))
    {
        PC->ShopComponent->Server_BuyItemFromShop(NewItem);
    }
}

void UM_ShopWidgetSlot::NativeConstruct()
{
    BuyButton->OnClicked.AddDynamic(this, &UM_ShopWidgetSlot::OnBuyButtonClicked);
}
