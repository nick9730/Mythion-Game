// Fill out your copyright notice in the Description page of Project Settings.


#include "M_QuestDialolgWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"  
#include "DataAsset/M_QuestDataAsset.h"
#include "Characters/PlayerCharacter.h"
#include "M_QuestComponent.h"
#include "M_QuestsInfos.h"
#include "M_PlayerController.h"
#include "AM_QuestNPC.h"

	/*
void UM_QuestDialolgWidget::InitDialog(AAM_QuestNPC* NPC, FQuestData QuestAsset)
{



	CurrentQuestAsset = QuestAsset;

	CurrentNPC = NPC;


	if (!QuestNameText || !QuestDescriptionText || !QuestRewardText) return;


	FString EnemyName = IsValid(QuestAsset.TargetEnemyClass) ? QuestAsset.TargetEnemyClass->GetName() : TEXT("Unknown");

	QuestNameText->SetText(FText::FromString(QuestAsset.QuestID));

	QuestDescriptionText->SetText(FText::FromString(FString::Printf(TEXT("Kill %d %s"), QuestAsset.RequiredKills, *QuestAsset.TargetEnemyClass->GetName())));

	QuestRewardText->SetText(FText::FromString(FString::Printf(TEXT("Reward: %d coins"), QuestAsset.CoinReward)));
}

void UM_QuestDialolgWidget::CloseDialog()
{
	SetIsDialogOpen(false);
	SetVisibility(ESlateVisibility::Collapsed);
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	PC->bShowMouseCursor = false;
	PC->SetInputMode(FInputModeGameOnly()); 



}

void UM_QuestDialolgWidget::OpenDialog()
{
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerCharacter)) return;

	UM_QuestComponent* QuestComponent = PlayerCharacter->FindComponentByClass<UM_QuestComponent>();
	if (!IsValid(QuestComponent)) return;
	

	SetIsDialogOpen(true);
	SetVisibility(ESlateVisibility::Visible);
	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	PC->SetInputMode(InputMode);
	
	if (AcceptButton)
		AcceptButton->OnClicked.AddDynamic(this, &UM_QuestDialolgWidget::OnAcceptClicked);

}


void UM_QuestDialolgWidget::OnAcceptClicked()
{


	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerCharacter)) return;
	UM_QuestComponent* QuestComponent = PlayerCharacter->FindComponentByClass<UM_QuestComponent>();
	if (!IsValid(QuestComponent)) return;

	QuestComponent->Server_AddQuest(CurrentQuestAsset);


	if (!IsValid(PC->QuestInfoWidget))
	{
		if (!PC->QuestInfoWidgetClass) return;

		PC->QuestInfoWidget = CreateWidget<UM_QuestsInfos>(PC, PC->QuestInfoWidgetClass);
		if (!IsValid(PC->QuestInfoWidget)) return;
		PC->QuestInfoWidget->AddToViewport();
		QuestComponent->OnQuestUpdated.AddDynamic(PC->QuestInfoWidget, &UM_QuestsInfos::UpdateQuest);
	}


	PC->QuestInfoWidget->UpdateQuest(CurrentQuestAsset, 0);
	PC->QuestInfoWidget->OpenQuestInfo();
	AddQuest(Quest);
	Quest.Status = EQuestStatus::Pending;
	BackEnd_SaveQuests(Quest);

	// Ενημέρωσε το UI του client
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!IsValid(PlayerCharacter)) return;
	AM_PlayerController* PC = Cast<AM_PlayerController>(PlayerCharacter->GetController());
	if (!IsValid(PC)) return;
	PC->Client_OnPendingQuestLoaded(Quest);
}

	*/

void UM_QuestDialolgWidget::InitDialog(AAM_QuestNPC* NPC, FQuestData Quest)
{
	CurrentQuest = Quest;
	CurrentNPC = NPC;

	if (!QuestNameText || !QuestDescriptionText || !QuestRewardText) return;

	FString EnemyName = IsValid(Quest.TargetEnemyClass) ? Quest.TargetEnemyClass->GetName() : TEXT("Unknown");

	QuestNameText->SetText(FText::FromString(Quest.QuestID));

	QuestDescriptionText->SetText(FText::FromString(FString::Printf(TEXT("Kill %d %s"), Quest.RequiredKills, *Quest.TargetEnemyClass->GetName())));

	QuestRewardText->SetText(FText::FromString(FString::Printf(TEXT("Reward: %d coins"), Quest.CoinReward)));

}

void UM_QuestDialolgWidget::CloseDialog()
{
	SetIsDialogOpen(false);
	SetVisibility(ESlateVisibility::Collapsed);
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	PC->bShowMouseCursor = false;
	PC->SetInputMode(FInputModeGameOnly());
}

void UM_QuestDialolgWidget::OpenDialog()
{
	SetIsDialogOpen(true);
	SetVisibility(ESlateVisibility::Visible);
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeGameAndUI());

}

void UM_QuestDialolgWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (AcceptButton)
	{
		AcceptButton->OnClicked.AddDynamic(this, &UM_QuestDialolgWidget::OnAcceptClicked);
	}
}

void UM_QuestDialolgWidget::OnAcceptClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[QUEST DIALOG] Accept button clicked for quest: %s"), *CurrentQuest.QuestID);

	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PC)) return;
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerCharacter)) return;
	UM_QuestComponent* QuestComp = PlayerCharacter->FindComponentByClass<UM_QuestComponent>();
	if (!IsValid(QuestComp)) return;

	QuestComp->Server_AddQuest(CurrentQuest);
	CloseDialog();
}
