// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "M_ShopWidgetSlot.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
class UButton;


UCLASS()
class MYTHION_API UM_ShopWidgetSlot : public UUserWidget
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* ItemImage;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemPrice;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemDescription;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemQuantity;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* BuyButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* SellButton;
    
	UPROPERTY()
	UM_Item_Details* CurrentItem;
	



	UFUNCTION()
	void InitSlot(UM_Item_Details* ItemDetails);

	UFUNCTION()
	void OnBuyButtonClicked();

	UFUNCTION()
	virtual void NativeConstruct() override;
	
};
