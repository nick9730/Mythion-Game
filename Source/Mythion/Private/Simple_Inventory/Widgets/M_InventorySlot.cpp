// Fill out your copyright notice in the Description page of Project Settings.



#include "Simple_Inventory/Widgets/M_InventorySlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Simple_Inventory/Widgets/M_HoverInfosItems.h"
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
        if (ItemIcon)
            ItemIcon->SetBrushFromTexture(ItemIconIdle);
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

void UM_InventorySlot::OpenQuantityWidget(FVector2D DesiredPosition)
{
    APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;
    AM_PlayerController* MPC = Cast<AM_PlayerController>(PC);   
    if (!IsValid(MPC)) return;
    FVector2D SlotPos = GetCachedGeometry().GetAbsolutePosition();
    MPC->Client_OpenQuantityWidget(ItemData, DesiredPosition, SlotIndex);

}

FReply UM_InventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && ItemData.IsValid())
    {           
            FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();

            
            FVector2D ViewportSize;
            GEngine->GameViewport->GetViewportSize(ViewportSize);

            FVector2D DesiredPosition = MousePosition;

            DesiredPosition.X = FMath::Clamp(DesiredPosition.X, 0.f, ViewportSize.X - 50.f);
            DesiredPosition.Y = FMath::Clamp(DesiredPosition.Y, 0.f, ViewportSize.Y - 50.f);
		
		OpenQuantityWidget(DesiredPosition);

        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }
    return FReply::Unhandled();
}

void UM_InventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (IsValid(PC) && IsValid(PC->QuantityWidget))
        PC->QuantityWidget->HideQuantityPopUp(ItemData);
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

    MPC->Server_DropItem(DraggedSlot->SlotIndex,ItemData.Quantity);

}

FReply UM_InventorySlot::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
     if (!ItemData.IsValid()) return FReply::Unhandled();

    AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (!IsValid(PC)) return FReply::Unhandled();

    PC->Server_UseItem(SlotIndex);
    return FReply::Handled();
}
void UM_InventorySlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (!ItemData.IsValid() || !IsValid(ItemData.ItemDataAsset) || !HoverInfosWidgetClass) return;

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

void UM_InventorySlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    //SetToolTip(nullptr);
}