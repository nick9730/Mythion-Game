// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/CharacterClasses.h"
#include "CharacterSelectionWidget.generated.h"

class UButton;
class UCharacterSelectDataAsset;
class UWidget;
class UM_SelectTag;
class UStatsWidgets;
class UM_SelectionStats;
class UTextBlock;
class UVerticalBox;
class UM_AbilitiesOfClasses;

class IHttpRequest;
class IHttpResponse;

using FHttpRequestPtr = TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>;
using FHttpResponsePtr = TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>;


UCLASS()
class MYTHION_API UCharacterSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
 UPROPERTY(meta=(BindWidget))
 UButton* NextButton;
 
 UPROPERTY(meta = (BindWidget))
 UButton* PreviousButton;


 UPROPERTY(meta = (BindWidget))
 UButton* ConfirmButton;


 UPROPERTY(meta = (BindWidget))
 UTextBlock* Description;



UFUNCTION(BlueprintCallable)
void OnConfirmButtonClicked();


 int32 CurrentCharacterIndex = 0;


 float Armor;
 float MagicResistance;
 float Health;
 float Mana;


 UFUNCTION()
 void OnNextButtonClicked();



 UFUNCTION()
 void OnPreviousButtonClicked();

 virtual void NativeConstruct() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Selection")
	AActor* PreviewActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Selection")
	UCharacterClasses* CharacterChoiceData;

	UFUNCTION(BlueprintCallable, Category = "Character Selection")
	void UpdateStatsDisplay(FCharacterClassData SelectedClassData);


	UPROPERTY(EditAnywhere,meta = (BindWidget))
	UM_SelectionStats* StatsWidget;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* AbilitiesContainer;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TSubclassOf<UM_AbilitiesOfClasses> AbilitiesInfos;

	UFUNCTION(BlueprintCallable, Category = "Character Selection")
	void CreateAbilitiesInfos(FCharacterClassData SelectedClassData);


	void OnConfirmComplete(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);

};
