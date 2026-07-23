// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_QuestsInfos.generated.h"

/**
 * 
 */
class UM_QuestDataAsset;
class UTextBlock;
struct FQuestData;

UCLASS()
class MYTHION_API UM_QuestsInfos : public UUserWidget
{
	GENERATED_BODY()


public:



    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestNameText;

    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestProgressText;

    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestCompleteText;

    void UpdateQuest(FQuestData Quest, int32 CurrentKills);
   // void ShowQuestComplete(FQuestData Quest);



	bool GetIsOpen() const { return bIsOpen; }
	void SetIsOpen(bool bOpen) { bIsOpen = bOpen; }


    void OpenQuestInfo();
	void CloseQuestInfo();

    virtual void NativeConstruct() override;


private:
	bool bIsOpen = false;

/*
*/
};
