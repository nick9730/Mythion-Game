// Fill out your copyright notice in the Description page of Project Settings.

#include "Simple_Inventory/Widgets/M_ArmorSlot.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_InventoryActionsComponent.h"
#include "PlayerController/Components/M_UIComponent.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "Simple_Inventory/Widgets/M_HoverInfosItems.h"
#include "Simple_Inventory/Widgets/M_QuantityWidget.h"

void UM_ArmorSlot::SetArmorSlot(const FItemData &Item)
{
    ItemData = Item;

    if (Item.IsValid())
    {
        if (ItemIcon && Item.Icon)
            ItemIcon->SetBrushFromTexture(Item.Icon);
    }
    else
    {
        ClearArmorSlot();
    }
}

void UM_ArmorSlot::ClearArmorSlot()
{
    ItemData = FItemData();

    if (ItemIcon)
        ItemIcon->SetBrushFromTexture(ItemIconIdle);
}

void UM_ArmorSlot::OpenQuantityWidget()
{
    APlayerController *PC = GetOwningPlayer();
    if (!IsValid(PC))
        return;
    AM_PlayerController *MPC = Cast<AM_PlayerController>(PC);
    if (!IsValid(MPC))
        return;
    FVector2D SlotPos = GetCachedGeometry().GetAbsolutePosition();
    MPC->UIComponent->Client_OpenQuantityWidget(ItemData, SlotPos, 1);
}

FReply UM_ArmorSlot::NativeOnMouseButtonDown(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && ItemData.IsValid())
    {
        OpenQuantityWidget();
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return FReply::Unhandled();
}

void UM_ArmorSlot::NativeOnDragDetected(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent,
                                        UDragDropOperation *&OutOperation)
{
    AM_PlayerController *PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (IsValid(PC) && IsValid(PC->UIComponent->QuantityWidget))
        PC->UIComponent->QuantityWidget->HideQuantityPopUp(ItemData);

    UDragDropOperation *DragOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    if (IsValid(DragOp))
    {
        DragOp->Payload = this;
        DragOp->DefaultDragVisual = this;
        OutOperation = DragOp;
    }
}

bool UM_ArmorSlot::NativeOnDrop(const FGeometry &InGeometry, const FDragDropEvent &InDragDropEvent,
                                UDragDropOperation *InOperation)
{
    UM_ArmorSlot *DraggedSlot = Cast<UM_ArmorSlot>(InOperation->Payload);
    if (!IsValid(DraggedSlot))
        return false;

    UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop called with DraggedSlot ItemID: %s"),
           *DraggedSlot->ItemData.ItemID.ToString());

    FItemData TempItem = ItemData;
    SetArmorSlot(DraggedSlot->ItemData);
    DraggedSlot->SetArmorSlot(TempItem);

    return true;
}

void UM_ArmorSlot::NativeOnDragCancelled(const FDragDropEvent &InDragDropEvent, UDragDropOperation *InOperation)
{
    UM_ArmorSlot *DraggedSlot = Cast<UM_ArmorSlot>(InOperation->Payload);
    if (!IsValid(DraggedSlot) || !DraggedSlot->ItemData.IsValid())
        return;

    if (DraggedSlot->ItemData.ItemType != EItemType::Armor)
        return;

    APlayerController *PC = GetOwningPlayer();
    AM_PlayerController *MPC = Cast<AM_PlayerController>(PC);
    if (!IsValid(MPC))
        return;

    MPC->InventoryActionsComponent->Server_DropArmorSlot();
}

FReply UM_ArmorSlot::NativeOnMouseButtonDoubleClick(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
    if (!ItemData.IsValid())
        return FReply::Unhandled();

    AM_PlayerController *PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (!IsValid(PC))
        return FReply::Unhandled();

    // PC->Server_UseArmorsSlot();
    return FReply::Handled();
}

void UM_ArmorSlot::NativeOnMouseEnter(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!ItemData.IsValid() || !IsValid(ItemData.ItemDataAsset) || !HoverInfosWidgetClass)
        return;

    if (!HoverInfosWidgetInstance)
    {
        HoverInfosWidgetInstance = CreateWidget<UM_HoverInfosItems>(GetOwningPlayer(), HoverInfosWidgetClass);
    }

    if (HoverInfosWidgetInstance)
    {
        HoverInfosWidgetInstance->InitHoverInfosItems(ItemData);
        SetToolTip(HoverInfosWidgetInstance);
    }
}

void UM_ArmorSlot::NativeOnMouseLeave(const FPointerEvent &InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    // SetToolTip(nullptr);
}