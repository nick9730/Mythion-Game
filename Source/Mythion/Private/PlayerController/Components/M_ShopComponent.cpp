// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_ShopComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "Shop/M_MerchantNPC.h"
#include "Shop/UI/M_ShopWidget.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Simple_Inventory/Widgets/M_Inventory.h"

#include "PlayerController/Components/M_InventoryActionsComponent.h"
#include "PlayerController/Components/M_UIComponent.h"
#include "Simple_Inventory/Widgets/M_QuantityWidget.h"

UM_ShopComponent::UM_ShopComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UM_ShopComponent::Client_OpenShop_Implementation(AM_MerchantNPC *Merchant)
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerPC);
    if (!IsValid(PCController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerPC->GetPawn());
    if (!IsValid(PlayerChar))
        return;

    if (OwnerPC->IsLocalController())
    {
        if (!IsValid(ShopWidget))
        {
            if (IsValid(ShopWidgetClass))
            {
                ShopWidget = CreateWidget<UM_ShopWidget>(OwnerPC, ShopWidgetClass);
                if (IsValid(ShopWidget))
                {
                    ShopWidget->AddToViewport(2);
                    ShopWidget->SetVisibility(ESlateVisibility::Hidden);
                }
            }
            else
            {
                return;
            }
        }
    }

    if (!IsValid(ShopWidget))
        return;

    if (!ShopWidget->bOpenShopUI)
    {
        ShopWidget->ShowShopUI();
        ShopWidget->InitShop(Merchant, PlayerChar->PlayerClassTag);

        if (IsValid(PCController->UIComponent->QuantityWidget))
        {
            PCController->UIComponent->QuantityWidget->bIsSellMode = true;
        }

        if (IsValid(GreetingSoundShop))
        {
            UGameplayStatics::PlaySoundAtLocation(this, GreetingSoundShop, PlayerChar->GetActorLocation());
        }

        if (IsValid(PCController->UIComponent->InventoryWidget))
        {
            PCController->UIComponent->InventoryWidget->SetVisibility(ESlateVisibility::Visible);
            OwnerPC->bShowMouseCursor = true;
            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            OwnerPC->SetInputMode(InputMode);
        }
    }
    else
    {
        ShopWidget->HideShopUI();
        if (IsValid(PCController->UIComponent->QuantityWidget))
            PCController->UIComponent->QuantityWidget->bIsSellMode = false;
        if (IsValid(PCController->UIComponent->InventoryWidget))
            PCController->UIComponent->InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
        OwnerPC->SetInputMode(FInputModeGameOnly());
        OwnerPC->bShowMouseCursor = false;
        if (IsValid(GoodByeSoundShop))
        {
            UGameplayStatics::PlaySoundAtLocation(this, GoodByeSoundShop, PlayerChar->GetActorLocation());
        }
    }
}

void UM_ShopComponent::Server_BuyItemFromShop_Implementation(FItemData Item)
{
    if (!Item.IsValid())
        return;

    APlayerController *OwnerController = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PC))
        return;

    UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

    int32 Coins = ASC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute());

    if (Coins < Item.ItemDataAsset->BuyPrice)
    {
        if (IsValid(PCController) && IsValid(PCController->InteractionTraceComponent))
            PCController->InteractionTraceComponent->Client_ShowNotification(FText::FromString("Not enough coins!"));
        return;
    }

    if (Item.ItemType == EItemType::Weapon)
    {
        if (PC->InventoryComponent->WeaponSlot.IsValid())
        {
            if (IsValid(PCController) && IsValid(PCController->InteractionTraceComponent))
                PCController->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("Weapon slot is full!")));
            return;
        }
        PC->InventoryComponent->Server_EquipItem(Item);
    }
    else if (Item.ItemType == EItemType::Armor)
    {
        if (PC->InventoryComponent->ArmorSlot.IsValid())
        {
            if (IsValid(PCController) && IsValid(PCController->InteractionTraceComponent))
                PCController->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("Armor slot is full!")));
            return;
        }
        PC->InventoryComponent->Server_EquipItem(Item);
    }
    else
    {
        if (!PC->InventoryComponent->HasSpace(Item))
        {
            if (IsValid(PCController) && IsValid(PCController->InteractionTraceComponent))
                PCController->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("Inventory is full!")));
            return;
        }
        PC->InventoryComponent->Server_AddItem(Item);
    }

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), -Item.ItemDataAsset->BuyPrice);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}