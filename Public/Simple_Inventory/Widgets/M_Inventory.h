#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "M_Inventory.generated.h"


class APlayerCharacter;

UCLASS()
class MYTHION_API UM_Inventory : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UUniformGridPanel* InventoryGrid;

    UPROPERTY(meta = (BindWidget))
    class UImage* WeaponSlotIcon;

    UPROPERTY(meta = (BindWidget))
    class UImage* ArmorSlotIcon;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UM_InventorySlot> InventorySlotClass;

    UPROPERTY(EditAnywhere)
    float InventoryOfWidthSlot;

    UPROPERTY(EditAnywhere)
    float InventoryOfHeightSlot;


    UFUNCTION()
    void UpdatePreviewTexture(UTextureRenderTarget2D* RT);

    UPROPERTY(meta = (BindWidget))
    class UImage* PreviewImage;

    UPROPERTY()
    APlayerCharacter* OwningCharacter;

    void SetOwningCharacter(APlayerCharacter* PC) { OwningCharacter = PC; }


    UPROPERTY(EditAnywhere)
    UMaterialInterface* PreviewMaterial;
 

    UPROPERTY()
    class UInventoryComponent* InventoryComp;

    void InitializeInventory(UInventoryComponent* InInventoryComp);
    void RefreshInventory();





protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void OnInventoryChanged();
};
