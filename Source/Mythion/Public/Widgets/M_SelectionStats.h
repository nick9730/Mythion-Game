// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_SelectionStats.generated.h"

/**
 * 
 */

class UProgressBar;
class UTextBlock;
class UImage;
class UWidget;


UCLASS()
class MYTHION_API UM_SelectionStats : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameText;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArmorValue;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ManaValue;


	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthValue;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagicResistanceValue;
	


};
