// Fill out your copyright notice in the Description page of Project Settings.


#include "M_QuestsInfos.h"
#include "Components/TextBlock.h"
#include "DataAsset/M_QuestDataAsset.h"



void UM_QuestsInfos::OpenQuestInfo()
{
    SetIsOpen(true);
	SetVisibility(ESlateVisibility::Visible);
}

void UM_QuestsInfos::CloseQuestInfo()
{
    SetIsOpen(false);
	SetVisibility(ESlateVisibility::Hidden);
}


void UM_QuestsInfos::UpdateQuest(FQuestData Quest, int32 CurrentKills)
{


    if (QuestNameText)
        QuestNameText->SetText(FText::FromString(Quest.QuestID));

    FString EnemyName = IsValid(Quest.TargetEnemyClass) ? Quest.TargetEnemyClass->GetName() : TEXT("enemies");

    if (QuestProgressText)
        QuestProgressText->SetText(FText::FromString(
            FString::Printf(TEXT("Kill %d/%d %s"), CurrentKills, Quest.RequiredKills, *EnemyName)));
}
