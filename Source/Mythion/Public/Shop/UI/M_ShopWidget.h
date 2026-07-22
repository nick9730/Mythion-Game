// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_ShopWidget.generated.h"

/**
 * 
 */
class UM_ShopWidgetSlot;
class UScrollBox;
struct FGameplayTag;

UCLASS()
class MYTHION_API UM_ShopWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void InitShop(AM_MerchantNPC* Merchant,FGameplayTag PlayerTag);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TSubclassOf<UM_ShopWidgetSlot> ShopWidgetSlotClass;

	UPROPERTY(BlueprintReadOnly)
	UM_ShopWidgetSlot* ShopWidgetSlot;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ShopGrid;

	void ShowShopUI();

	void HideShopUI();

	bool bOpenShopUI = true;


	void  SetOpenShopUI(bool bOpen) { bOpenShopUI = bOpen; }
private:

	
};
