// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerWidget/M_ShowNotification.h"
#include "Components/TextBlock.h"


void UM_ShowNotification::ShowMessage(const FText& Message)
{
    bIsShowingSystemMessage = true;
    if (MessageText)
        MessageText->SetText(Message);

    SetVisibility(ESlateVisibility::Visible);

   
    GetWorld()->GetTimerManager().ClearTimer(HideTimer);

    GetWorld()->GetTimerManager().SetTimer(HideTimer, [this]()
        {
            SetVisibility(ESlateVisibility::Hidden);
            bIsShowingSystemMessage = false;
        }, 3.f, false);
}

void UM_ShowNotification::ShowNameOfItem(const FText& ItemName)
{
    if (bIsShowingSystemMessage) return;

    if (MessageText)
    {
        MessageText->SetText(ItemName);
    }
    SetVisibility(ESlateVisibility::Visible);
    GetWorld()->GetTimerManager().ClearTimer(HideTimer);
}

void UM_ShowNotification::HideNotification()
{
    SetVisibility(ESlateVisibility::Hidden);
    bIsShowingSystemMessage = false;
}