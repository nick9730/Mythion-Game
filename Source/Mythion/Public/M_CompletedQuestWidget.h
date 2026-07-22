// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "M_CompletedQuestWidget.generated.h"

/**
 * 
 */


UCLASS()
class MYTHION_API UM_CompletedQuestWidget : public UUserWidget
{
	GENERATED_BODY()
	

	public:
		UPROPERTY(meta = (BindWidget))
		class UTextBlock* QuestCompletedText;

		UFUNCTION()
		void SetText(FQuestData Quest);
	

		virtual void NativeConstruct() override;
};
