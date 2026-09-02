// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "M_UIComponent.generated.h"

class UM_Inventory;
class UM_QuantityWidget;
class UUserWidget;
class USoundBase;
struct FItemData;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_UIComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_UIComponent();

    void TryInitInventory();
    void ToggleInventory();
    void OnInventoryFull();

    UFUNCTION(Client, Reliable)
    void Client_OpenQuantityWidget(FItemData ItemData, FVector2D SlotPosition, int32 SlotIndex);

    UFUNCTION(Client, Reliable)
    void Client_ShowOptionsWidget();

    UFUNCTION(Client, Reliable)
    void Client_HideOptionsWidget();

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<UM_Inventory> InventoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<UUserWidget> OptionsWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> Sound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    TObjectPtr<USoundBase> CloseSound;

    UPROPERTY()
    TObjectPtr<UM_Inventory> InventoryWidget;

    UPROPERTY()
    TObjectPtr<UM_QuantityWidget> QuantityWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Quantity")
    TSubclassOf<UM_QuantityWidget> QuantityWidgetClass;

    FTimerHandle InventoryInitTimer;

  private:
    UPROPERTY()
    TObjectPtr<UUserWidget> OptionsWidgetInstance;
};
