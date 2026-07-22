// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_ShowNotification.generated.h"

/**
 * 
 */

class UTextBlock;
class FText;


UCLASS()
class MYTHION_API UM_ShowNotification : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UPROPERTY(meta = (BindWidget))
	 UTextBlock* MessageText;

	UFUNCTION(BlueprintCallable)
	void ShowMessage(const FText& Message);

	UFUNCTION(BlueprintCallable)
	void ShowNameOfItem(const FText& ItemName);

	FTimerHandle HideTimer;

	UFUNCTION(BlueprintCallable)
	void HideNotification();

	bool bIsShowingSystemMessage = false;
};
