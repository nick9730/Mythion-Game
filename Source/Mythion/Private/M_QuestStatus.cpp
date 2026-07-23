// Fill out your copyright notice in the Description page of Project Settings.


#include "M_QuestStatus.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "Components/WidgetComponent.h"
#include "AM_QuestNPC.h"

void UM_QuestStatus::SetQuestStatusImage(UTexture2D* NewTexture)
{
	if (QuestStatusImage)
	{
		QuestStatusImage->SetBrushFromTexture(NewTexture);
	}
}

void UM_QuestStatus::UpdateQuestStatusImage(EQuestStatus Status)
{
	CurrentStatus = Status;

		switch (Status)
		{
		case EQuestStatus::NotTaken:
			SetQuestStatusImage(NotTakenImage);
			break;
		case EQuestStatus::Pending:
			SetQuestStatusImage(PendingImage);
			break;
		case EQuestStatus::Completed:
			SetQuestStatusImage(CompletedImage);
			break;
		default:
			break;
		
	}
}

void UM_QuestStatus::NativeConstruct()
{
	Super::NativeConstruct();


	if (UWidgetComponent* OwningComponent = Cast<UWidgetComponent>(GetOuter()))
	{
		if (AAM_QuestNPC* NPC = Cast<AAM_QuestNPC>(OwningComponent->GetOwner()))
		{
			CurrentStatus = NPC->CurrentQuestStatus;
		}
	}

	UpdateQuestStatusImage(CurrentStatus);

}

