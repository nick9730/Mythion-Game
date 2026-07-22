// Fill out your copyright notice in the Description page of Project Settings.



#include "Simple_Inventory/Widgets/M_InventorySlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "M_PlayerController.h"

void UM_InventorySlot::SetItem(const FItemData& Item, int32 Index)
{




    ItemData = Item;
    SlotIndex = Index;

    if (Item.IsValid())
    {
        if (ItemIcon && Item.Icon)
            ItemIcon->SetBrushFromTexture(Item.Icon);

        if (QuantityText)
        {
            if (Item.Quantity > 1)
            {
                QuantityText->SetText(FText::AsNumber(Item.Quantity));
                QuantityText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                QuantityText->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
    else
    {
        ClearSlot();
    }
}

void UM_InventorySlot::ClearSlot()
{
    ItemData = FItemData();

    if (ItemIcon)
        ItemIcon->SetBrushFromTexture(nullptr);

    if (QuantityText)
        QuantityText->SetVisibility(ESlateVisibility::Hidden);
}

FReply UM_InventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && ItemData.IsValid())
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return FReply::Unhandled();
}

void UM_InventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    UDragDropOperation* DragOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());
    if (IsValid(DragOp))
    {
        DragOp->Payload = this;
        DragOp->DefaultDragVisual = this;
        OutOperation = DragOp;
    }
}

bool UM_InventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UM_InventorySlot* DraggedSlot = Cast<UM_InventorySlot>(InOperation->Payload);
    if (!IsValid(DraggedSlot)) return false;

  
    FItemData TempItem = ItemData;
    SetItem(DraggedSlot->ItemData, SlotIndex);
    DraggedSlot->SetItem(TempItem, DraggedSlot->SlotIndex);

    return true;
}

void UM_InventorySlot::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UM_InventorySlot* DraggedSlot = Cast<UM_InventorySlot>(InOperation->Payload);
    if (!IsValid(DraggedSlot) || !DraggedSlot->ItemData.IsValid()) return;

  
    APlayerController* PC = GetOwningPlayer();
    AM_PlayerController* MPC = Cast<AM_PlayerController>(PC);
    if (!IsValid(MPC)) return;

    MPC->Server_DropItem(DraggedSlot->SlotIndex);

}

FReply UM_InventorySlot::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
     if (!ItemData.IsValid()) return FReply::Unhandled();

    AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (!IsValid(PC)) return FReply::Unhandled();

    PC->Server_UseItem(SlotIndex);
    return FReply::Handled();
}
