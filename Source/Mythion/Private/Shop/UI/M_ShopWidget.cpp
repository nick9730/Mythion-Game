// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop/UI/M_ShopWidget.h"
#include "M_PlayerController.h"
#include  "Components/UniformGridPanel.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Shop/UI/M_ShopWidgetSlot.h"
#include "Shop/M_MerchantNPC.h"
#include "Components/ScrollBox.h"

void UM_ShopWidget::NativeConstruct()
{
}

void UM_ShopWidget::InitShop(AM_MerchantNPC* Merchant,FGameplayTag Tag)
{
    if (!IsValid(Merchant) || !IsValid(ShopGrid) || !IsValid(ShopWidgetSlotClass)) return;

	ShopGrid->ClearChildren();


	TArray<UM_Item_Details*> FilteredItems = Merchant->GetFilteredShopItems(Tag);

	for (UM_Item_Details* Item : FilteredItems)
	{
		if (!IsValid(Item)) continue;

		UM_ShopWidgetSlot* ShopSlot = CreateWidget<UM_ShopWidgetSlot>(GetOwningPlayer(), ShopWidgetSlotClass);
		if (!IsValid(ShopSlot)) continue;

		ShopSlot->InitSlot(Item);
		ShopGrid->AddChild(ShopSlot);
	}

}

void UM_ShopWidget::ShowShopUI()
{
	SetOpenShopUI(true);
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if(!IsValid(PC)) return ;
	
	FInputModeGameAndUI InputMode;
	PC->SetInputMode(InputMode);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PC->bShowMouseCursor = true;



}

void UM_ShopWidget::HideShopUI()
{
	SetOpenShopUI(false);
   AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;

	SetVisibility(ESlateVisibility::Collapsed);
	PC->SetInputMode(FInputModeGameOnly());
	PC->bShowMouseCursor = false;
}
