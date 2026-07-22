// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/Widgets/M_Inventory.h"
#include "Components/Image.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Simple_Inventory/Widgets/M_InventorySlot.h"
#include "Net/UnrealNetwork.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Characters/PlayerCharacter.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"


void UM_Inventory::NativeConstruct()
{
    Super::NativeConstruct();

    TryBindASC();
}



void UM_Inventory::UpdatePreviewTexture(UTextureRenderTarget2D* RT)
{
    if (!IsValid(RT)) return;

    FSlateBrush Brush;
    Brush.SetResourceObject(RT);
    Brush.ImageSize = FVector2D(256.f, 256.f);


}

void UM_Inventory::InitializeInventory(UInventoryComponent* InInventoryComp)
{
    if (!IsValid(InInventoryComp)) return;

    InventoryComp = InInventoryComp;
    InventoryComp->OnInventoryChanged.AddDynamic(this, &UM_Inventory::OnInventoryChanged);

    RefreshInventory();
}

void UM_Inventory::RefreshInventory()
{
    if (!IsValid(InventoryComp) || !InventoryGrid || !InventorySlotClass) return;

    InventoryGrid->ClearChildren();

    for (int32 i = 0; i < InventoryComp->Items.Num(); i++)
    {


        UM_InventorySlot* InventorySlot = CreateWidget<UM_InventorySlot>(GetOwningPlayer(), InventorySlotClass);
        if (!IsValid(InventorySlot)) continue;

        InventorySlot->SetItem(InventoryComp->Items[i], i);

        UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(InventorySlot);

       

        if (GridSlot)
        {
            GridSlot->SetColumn(i%8);
            GridSlot->SetRow(i/8);
		
        }
    }

 
    //if (WeaponSlotIcon && InventoryComp->WeaponSlot.IsValid())
       // WeaponSlotIcon->SetBrushFromTexture(InventoryComp->WeaponSlot.Icon);

    if (IsValid(WeaponSlot) && IsValid(InventoryComp))
    {
        WeaponSlot->SetWeapon(InventoryComp->WeaponSlot);
    }

	
    if (IsValid(WeaponSlot) && IsValid(InventoryComp))
    {
        ArmorSlot->SetArmorSlot(InventoryComp->ArmorSlot);
    }


    UpdateCoinText();
    UpdateArmorText();
    UpdateMagicResistanceText();
}

void UM_Inventory::BindToASC(UAbilitySystemComponent* ASC)
{

  CurrentCoins =   ASC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute());
  CurrentArmor = ASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());
  CurrentMagicResistance = ASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());


  CoinHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetCoinsAttribute()).AddUObject(this, &UM_Inventory::OnCoinsChanged);
  ArmorHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetArmorAttribute()).AddUObject(this, &UM_Inventory::OnArmorChanged);
  MagicResistanceHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMagicResistanceAttribute()).AddUObject(this, &UM_Inventory::OnMagicResistanceChanged);

  UpdateCoinText();
  UpdateArmorText();
  UpdateMagicResistanceText();
}

FReply UM_Inventory::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        bIsDragging = true;
        DragOffset = InMouseEvent.GetScreenSpacePosition() - GetCachedGeometry().GetAbsolutePosition();
        return FReply::Handled().CaptureMouse(GetCachedWidget().ToSharedRef());
    }
    return FReply::Unhandled();
}

FReply UM_Inventory::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging)
    {
        FVector2D NewPos = InMouseEvent.GetScreenSpacePosition() - DragOffset;
        SetPositionInViewport(NewPos);
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply UM_Inventory::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    bIsDragging = false;
    return FReply::Handled().ReleaseMouseCapture();
}



void UM_Inventory::OnInventoryChanged()
{
    RefreshInventory();
}

void UM_Inventory::OnCoinsChanged(const FOnAttributeChangeData& Data)
{
    CurrentCoins = Data.NewValue;

    UpdateCoinText();
}

void UM_Inventory::OnArmorChanged(const FOnAttributeChangeData& Data)
{
	CurrentArmor = Data.NewValue;
   
    UpdateArmorText();
}

void UM_Inventory::OnMagicResistanceChanged(const FOnAttributeChangeData& Data)
{
	CurrentMagicResistance = Data.NewValue;
    UpdateMagicResistanceText();
}

void UM_Inventory::UpdateCoinText()
{
    if (CoinText)
	CoinText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentCoins))));


}

void UM_Inventory::UpdateArmorText()
{
    if (ArmorText)
		ArmorText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentArmor))));
}

void UM_Inventory::UpdateMagicResistanceText()
{
    if (MagicResistanceText)
		MagicResistanceText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::FloorToInt(CurrentMagicResistance))));
}


void UM_Inventory::TryBindASC()
{
   
    APawn* Pawn = GetOwningPlayer() ? GetOwningPlayer()->GetPawn() : nullptr;
    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Pawn);
    UAbilitySystemComponent* ASC = PlayerChar ? PlayerChar->GetAbilitySystemComponent() : nullptr;

    if (IsValid(ASC))
    {
        BindToASC(ASC);
        GetWorld()->GetTimerManager().ClearTimer(BindASCTimerHandle);
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(
            BindASCTimerHandle,
            this,
            &UM_Inventory::TryBindASC,
            0.2f,
            false
        );
    }
}