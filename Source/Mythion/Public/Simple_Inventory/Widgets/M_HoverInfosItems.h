// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_HoverInfosItems.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;
struct FItemData;	

UCLASS()
class MYTHION_API UM_HoverInfosItems : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UFUNCTION(BlueprintCallable)
	void InitHoverInfosItems(const FItemData& Item);
	
	
};
