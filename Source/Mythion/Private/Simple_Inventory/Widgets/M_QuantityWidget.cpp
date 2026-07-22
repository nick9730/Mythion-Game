// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/Widgets/M_QuantityWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "M_PlayerController.h"



void UM_QuantityWidget::NativeConstruct()
{
	bool bIsEquipmentSlot = (CurrentItem.ItemType == EItemType::Weapon || CurrentItem.ItemType == EItemType::Armor);


	if (IncreaseButton) IncreaseButton->SetVisibility(bIsEquipmentSlot ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (DecreaseButton) DecreaseButton->SetVisibility(bIsEquipmentSlot ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (QuantityText) QuantityText->SetVisibility(bIsEquipmentSlot ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	if (UseButton) UseButton->SetVisibility(bIsEquipmentSlot ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);


	if (IncreaseButton) IncreaseButton->OnClicked.AddDynamic(this, &UM_QuantityWidget::IncreaseQuantity);
	if (DecreaseButton) DecreaseButton->OnClicked.AddDynamic(this, &UM_QuantityWidget::DecreaseQuantity);
	if (CancelButton) CancelButton->OnClicked.AddDynamic(this, &UM_QuantityWidget::CancelQuantity);
	if (ConfirmButton) ConfirmButton->OnClicked.AddDynamic(this, &UM_QuantityWidget::ConfirmQuantity);
	if (UseButton) UseButton->OnClicked.AddDynamic(this, &UM_QuantityWidget::UseItems);

}

void UM_QuantityWidget::SetQuantity(int32 NewQuantity)
{
	CurrentQuantity = NewQuantity;
}

void UM_QuantityWidget::IncreaseQuantity()
{
	if (CurrentQuantity <MaxQuantity )
	{
		CurrentQuantity++;
		if (QuantityText)
			QuantityText->SetText(FText::FromString(FString::FromInt(CurrentQuantity)));
	}
}

void UM_QuantityWidget::DecreaseQuantity()
{
	if (CurrentQuantity > 1)
	{
		CurrentQuantity--;
		if (QuantityText)
			QuantityText->SetText(FText::FromString(FString::FromInt(CurrentQuantity)));
	}
}

void UM_QuantityWidget::ConfirmQuantity()
{
	SetVisibility(ESlateVisibility::Hidden);
	SetbIsOpen(false);

	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;


	if (bIsSellMode) {
	
		if (CurrentItem.ItemType == EItemType::Weapon) {
			PC->Server_SellWeapon();
		}
		else if (CurrentItem.ItemType == EItemType::Armor) {
			PC->Server_SellArmor();
		}
		else {

			PC->Server_SellItem(SelectedIndex, CurrentQuantity);

		}
	}
	else {
		if (CurrentItem.ItemType == EItemType::Weapon) {
			PC->Server_DropWeaponSlot();
		}
		else if (CurrentItem.ItemType == EItemType::Armor) {
			PC->Server_DropArmorSlot();
		}
		else {

		PC->Server_DropItem(SelectedIndex, CurrentQuantity);
		}


	}

}
void UM_QuantityWidget::UseItems() {

	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	PC->Server_UseItem(SelectedIndex);

}

void UM_QuantityWidget::CancelQuantity()
{
	SetVisibility(ESlateVisibility::Hidden);
	SetbIsOpen(false);
}

void UM_QuantityWidget::ShowQuantityPopUp(FItemData Item,FVector2D SlotPosition,int32 SlotIndex)
{
	
	CurrentItem = Item;
	SelectedIndex = SlotIndex;
	CurrentQuantity = 1;
	MaxQuantity = Item.Quantity;

	if (QuantityText)
		QuantityText->SetText(FText::FromString(FString::FromInt(CurrentQuantity)));
	if (DropSellText)
		DropSellText->SetText(bIsSellMode ? FText::FromString(TEXT("Sell")) : FText::FromString(TEXT("Drop")));
	SetPositionInViewport(FVector2D(SlotPosition.X-10.0, SlotPosition.Y-30.0));
	SetVisibility(ESlateVisibility::Visible);	
	SetbIsOpen(true);

}

void UM_QuantityWidget::HideQuantityPopUp(FItemData Item)
{
	SetVisibility(ESlateVisibility::Hidden);
	SetbIsOpen(false);

}

