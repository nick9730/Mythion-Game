// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Blueprint/UserWidget.h"
#include "M_QuantityWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;

UCLASS()
class MYTHION_API UM_QuantityWidget : public UUserWidget
{
	GENERATED_BODY()


public:

	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UTextBlock* QuantityText;
	 
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* DropSellText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UButton* IncreaseButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UButton* DecreaseButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UButton* ConfirmButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UButton* CancelButton;

	 UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	 UButton* UseButton;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentQuantity;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxQuantity;

	FItemData CurrentItem;

	UFUNCTION(BlueprintCallable)
	void SetQuantity(int32 NewQuantity);
	UFUNCTION(BlueprintCallable)
	void IncreaseQuantity();
	UFUNCTION(BlueprintCallable)
	void DecreaseQuantity();
	UFUNCTION(BlueprintCallable)
	void ConfirmQuantity();
	UFUNCTION(BlueprintCallable)
	void CancelQuantity();

	UFUNCTION(BlueprintCallable)
	void UseItems();

	int32 SelectedIndex = -1;

	UFUNCTION()
	void ShowQuantityPopUp(FItemData Item, FVector2D SlotPosition, int32 SlotIndex);

	UFUNCTION()
	void HideQuantityPopUp(FItemData Item);


	bool bIsSellMode = false;
	bool bIsOpen = false;

	void SetbIsOpen(bool bOpen) { bIsOpen = bOpen; }
private:

	
};
