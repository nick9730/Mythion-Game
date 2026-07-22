#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "GameplayEffectTypes.h"
#include "Simple_Inventory/Widgets/M_WeaponSlot.h"
#include "Simple_Inventory/Widgets/M_ArmorSlot.h"

#include "M_Inventory.generated.h"


class APlayerCharacter;
class AbilitySystemComponent;
class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
struct FGameplayEventData;
struct FGameplayTag;
class UUniformGridPanel;
class UImage;
class UInventoryComponent;
class TextBlock;


UCLASS()
class MYTHION_API UM_Inventory : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;



    UPROPERTY(meta = (BindWidget))
    UImage* ArmorSlotIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoinText;

    UPROPERTY(meta = (BindWidget))
	UTextBlock* ArmorText;

    UPROPERTY(meta = (BindWidget))
	UTextBlock* MagicResistanceText;


    UPROPERTY(meta = (BindWidget))
    UM_WeaponSlot* WeaponSlot;

    UPROPERTY(meta = (BindWidget))
    UM_ArmorSlot*  ArmorSlot;


    UPROPERTY(EditAnywhere)
    TSubclassOf<class UM_InventorySlot> InventorySlotClass;

    UPROPERTY(EditAnywhere)
    float InventoryOfWidthSlot;

    UPROPERTY(EditAnywhere)
    float InventoryOfHeightSlot;


    void UpdateCoinText();
    void UpdateArmorText();
    void UpdateMagicResistanceText();

    FTimerHandle BindASCTimerHandle;

   UFUNCTION()
   void TryBindASC();

    UFUNCTION()
    void UpdatePreviewTexture(UTextureRenderTarget2D* RT);



    UPROPERTY()
    APlayerCharacter* OwningCharacter;

    void SetOwningCharacter(APlayerCharacter* PC) { OwningCharacter = PC; }


    UPROPERTY(EditAnywhere)
    UMaterialInterface* PreviewMaterial;
 

    UPROPERTY()
    class UInventoryComponent* InventoryComp;

    void InitializeInventory(UInventoryComponent* InInventoryComp);
    void RefreshInventory();

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> BoundASC;

    void BindToASC(UAbilitySystemComponent* ASC);

    FDelegateHandle ArmorHandle;
    FDelegateHandle MagicResistanceHandle;
    FDelegateHandle CoinHandle;


    // Drag
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    bool bIsDragging = false;
    FVector2D DragOffset;
   



protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void OnInventoryChanged();

    void OnCoinsChanged(const FOnAttributeChangeData& Data);
    void OnArmorChanged(const FOnAttributeChangeData& Data);
    void OnMagicResistanceChanged(const FOnAttributeChangeData& Data);




    float  CurrentArmor= 0.f;
    float CurrentMagicResistance = 1.f;
    float CurrentCoins = 1.f;

};
