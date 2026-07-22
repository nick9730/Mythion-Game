// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "M_ArmorSlot.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API UM_ArmorSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIcon;


	UPROPERTY(EditAnywhere, Category = "IdleIcon")
	class UTexture2D* ItemIconIdle;

	UPROPERTY(BlueprintReadWrite)
	FItemData ItemData;

	void SetArmorSlot(const FItemData& Item);

	void ClearArmorSlot();


	UFUNCTION()
	void OpenQuantityWidget();

	UPROPERTY(EditDefaultsOnly, Category = "HoverInfos")
	TSubclassOf<class UM_HoverInfosItems> HoverInfosWidgetClass;

	UPROPERTY()
	class UM_HoverInfosItems* HoverInfosWidgetInstance;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

};
