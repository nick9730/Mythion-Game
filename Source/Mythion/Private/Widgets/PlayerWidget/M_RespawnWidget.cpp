// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/PlayerWidget/M_RespawnWidget.h"
#include "Components/Button.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_RespawnComponent.h"

void UM_RespawnWidget::OnRespawnClicked()
{
    AM_PlayerController *PC = Cast<AM_PlayerController>(GetOwningPlayer());
    if (!IsValid(PC))
        return;

    PC->RespawnComponent->Server_Respawn();
}

void UM_RespawnWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (RespawnButton)
        RespawnButton->OnClicked.AddDynamic(this, &UM_RespawnWidget::OnRespawnClicked);
}
