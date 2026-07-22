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



void UM_Inventory::NativeConstruct()
{
    Super::NativeConstruct();

    APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetOwningPlayer()->GetPawn());
    if (!IsValid(PlayerChar)) return;


    if (IsValid(PlayerChar->PreviewRenderTarget))
    {
        UpdatePreviewTexture(PlayerChar->PreviewRenderTarget);
        return;
    }

 
    PlayerChar->OnPreviewReady.AddDynamic(this, &UM_Inventory::UpdatePreviewTexture);
}



void UM_Inventory::UpdatePreviewTexture(UTextureRenderTarget2D* RT)
{
    if (!IsValid(RT)) return;


    UMaterialInstanceDynamic* DynMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, PreviewMaterial);
    if (!IsValid(DynMat)) return;

    DynMat->SetTextureParameterValue(FName("Texture"), RT);
    PreviewImage->SetBrushFromMaterial(DynMat);

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

 
    if (WeaponSlotIcon && InventoryComp->WeaponSlot.IsValid())
        WeaponSlotIcon->SetBrushFromTexture(InventoryComp->WeaponSlot.Icon);

    if (ArmorSlotIcon && InventoryComp->ArmorSlot.IsValid())
        ArmorSlotIcon->SetBrushFromTexture(InventoryComp->ArmorSlot.Icon);
}

void UM_Inventory::OnInventoryChanged()
{
    RefreshInventory();
}