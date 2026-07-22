// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatsWidgets.generated.h"

struct FCharacterChoiceData;
class UProgressBar;
class UTextBlock;

UCLASS()
class MYTHION_API UStatsWidgets : public UUserWidget
{
	GENERATED_BODY()
	
public:



	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category = "Stats")
	void UpdateStatsDisplay(FCharacterChoiceData SelectedData);

	



};
