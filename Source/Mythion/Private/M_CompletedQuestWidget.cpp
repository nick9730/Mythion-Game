// Fill out your copyright notice in the Description page of Project Settings.


#include "M_CompletedQuestWidget.h"
#include "Components/TextBlock.h"	

void UM_CompletedQuestWidget::SetText(FQuestData Quest)
{
	if (QuestCompletedText)
	{
		QuestCompletedText->SetText(FText::FromString(FString::Printf(TEXT("Congrats, You have earned: %d coins"), Quest.CoinReward)));
	}
}

void UM_CompletedQuestWidget::NativeConstruct()
{

}


