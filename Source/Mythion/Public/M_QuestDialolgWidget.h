// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "M_QuestDialolgWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;
class UM_QuestDataAsset;
struct FQuestData;
class AAM_QuestNPC;


UCLASS()
class MYTHION_API UM_QuestDialolgWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestNameText;

    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestDescriptionText;

    UPROPERTY(meta = (BindWidget))
     UTextBlock* QuestRewardText;

    UPROPERTY(meta = (BindWidget))
     UButton* AcceptButton;

    UPROPERTY()
    FQuestData CurrentQuest;

    UPROPERTY()
    FString CurrentQuestID;

    UPROPERTY()
	AAM_QuestNPC* CurrentNPC;


    void InitDialog(AAM_QuestNPC* NPC,FQuestData Quest);

    void CloseDialog();
    void OpenDialog();
    
	bool GetIsDialogOpen() const { return bIsDialogOpen; }
	void SetIsDialogOpen(bool bOpen) { bIsDialogOpen = bOpen; }
protected:
    virtual void NativeConstruct() override;


    UFUNCTION()
    void OnAcceptClicked();

private : 
	bool bIsDialogOpen = false;
	


};