// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "CharacterSelectionWidget.generated.h"

class UButton;
class UCharacterSelectDataAsset;
class UWidget;
class UM_SelectTag;

class UStatsWidgets;

UCLASS()
class MYTHION_API UCharacterSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
 UPROPERTY(meta=(BindWidget))
 UButton* NextButton;

 UPROPERTY(meta = (BindWidget))
 UButton* PreviousButton;


 int32 CurrentCharacterIndex = 0;

 UFUNCTION()
 void OnNextButtonClicked();


 UFUNCTION()
 void OnPreviousButtonClicked();

 virtual void NativeConstruct() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Selection")
	UCharacterSelectDataAsset* CharacterChoiceData;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character Selection")
	void OnCharacterChanged(FCharacterChoiceData SelectedData);




};
