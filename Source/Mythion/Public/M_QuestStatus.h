// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "M_QuestStatus.generated.h"

/**
 * 
 */

class UImage;
class UTexture2D;
struct FQuestData;


UCLASS()
class MYTHION_API UM_QuestStatus : public UUserWidget
{
	GENERATED_BODY()
	
public:



public:


   virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UImage* QuestStatusImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestStatus")
    UTexture2D* NotTakenImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestStatus")
    UTexture2D* PendingImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestStatus")
    UTexture2D* CompletedImage;

    UPROPERTY()
    FQuestData Quest;

    UPROPERTY()
    EQuestStatus CurrentStatus;


    UFUNCTION()
    void SetQuestStatusImage(UTexture2D* NewTexture);

    UFUNCTION()
    void UpdateQuestStatusImage(EQuestStatus Status);




};
