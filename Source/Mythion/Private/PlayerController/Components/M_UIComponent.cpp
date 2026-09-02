// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_UIComponent.h"
#include "AbilitySystemComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "GameFramework/Controller.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Simple_Inventory/Widgets/M_Inventory.h"
#include "Simple_Inventory/Widgets/M_QuantityWidget.h"

UM_UIComponent::UM_UIComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UM_UIComponent::TryInitInventory()
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerPC->GetPawn());
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
        return;

    GetWorld()->GetTimerManager().ClearTimer(InventoryInitTimer);

    if (IsValid(InventoryWidgetClass))
    {
        InventoryWidget = CreateWidget<UM_Inventory>(OwnerPC, InventoryWidgetClass);
        if (IsValid(InventoryWidget))
        {
            InventoryWidget->InitializeInventory(PC->InventoryComponent);
            InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
            InventoryWidget->AddToViewport();
        }
    }
}

void UM_UIComponent::Client_OpenQuantityWidget_Implementation(FItemData ItemData, FVector2D SlotPosition,
                                                              int32 SlotIndex)
{
    if (!IsValid(QuantityWidget))
        return;

    if (!QuantityWidget->bIsOpen)
        QuantityWidget->ShowQuantityPopUp(ItemData, SlotPosition, SlotIndex);
    else
        QuantityWidget->HideQuantityPopUp(ItemData);
}

void UM_UIComponent::Client_ShowOptionsWidget_Implementation()
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (!OptionsWidgetClass)
        return;

    if (!OptionsWidgetInstance)
    {
        OptionsWidgetInstance = CreateWidget<UUserWidget>(OwnerPC, OptionsWidgetClass);
    }

    if (OptionsWidgetInstance && !OptionsWidgetInstance->IsInViewport())
    {
        OptionsWidgetInstance->AddToViewport();
    }

    OwnerPC->SetInputMode(FInputModeGameAndUI());
    OwnerPC->SetShowMouseCursor(true);
}

void UM_UIComponent::Client_HideOptionsWidget_Implementation()
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (OptionsWidgetInstance && OptionsWidgetInstance->IsInViewport())
    {
        OptionsWidgetInstance->RemoveFromParent();
    }

    OwnerPC->SetInputMode(FInputModeGameOnly());
    OwnerPC->SetShowMouseCursor(false);
}

void UM_UIComponent::ToggleInventory()
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC) || !OwnerPC->IsLocalController())
        return;

    if (!IsValid(InventoryWidget))
        return;

    bool bIsVisible = InventoryWidget->GetVisibility() == ESlateVisibility::Visible;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerPC->GetPawn());
    if (!IsValid(PC))
        return;

    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    if (!IsValid(PS))
        return;

    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    if (bIsVisible)
    {
        InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
        OwnerPC->SetInputMode(FInputModeGameOnly());
        OwnerPC->bShowMouseCursor = false;

        if (!IsValid(QuantityWidget))
            return;
        QuantityWidget->SetbIsOpen(false);
        QuantityWidget->SetVisibility(ESlateVisibility::Hidden);
        OwnerPC->ClientPlaySoundAtLocation(CloseSound, PC->GetActorLocation(), 1, 1);
    }
    else
    {
        InventoryWidget->SetVisibility(ESlateVisibility::Visible);
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        OwnerPC->ClientPlaySoundAtLocation(Sound, PC->GetActorLocation(), 1, 1);
        OwnerPC->SetInputMode(InputMode);
        OwnerPC->bShowMouseCursor = true;
    }
}

void UM_UIComponent::OnInventoryFull()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Inventory is full!"));
}