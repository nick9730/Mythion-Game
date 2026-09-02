// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Simple_Inventory/Data/M_Item_Details.h"

#include "M_InventoryActionsComponent.generated.h"

class AM_Interactive_Item;
class UAbilitySystemComponent;
class UGameplayEffect;

enum class EItemStats : uint8;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_InventoryActionsComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_InventoryActionsComponent();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UpdateInventoryAfterLoad(const TArray<FItemData> &LoadedItems, FItemData WeaponItem,
                                         FItemData ArmorItem);

    UFUNCTION(Server, Reliable)
    void Server_DropItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_SellItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_SellWeapon();

    UFUNCTION(Server, Reliable)
    void Server_SellArmor();

    UFUNCTION(Server, Reliable)
    void Server_UseItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_DropWeaponSlot();

    UFUNCTION(Server, Reliable)
    void Server_UseWeaponSlot();

    UFUNCTION(Server, Reliable)
    void Server_DropArmorSlot();

    UFUNCTION(Server, Reliable)
    void Server_UseArmorsSlot();

    UPROPERTY(EditDefaultsOnly, Category = "Economy")
    TSubclassOf<UGameplayEffect> CoinEffect;

  private:
    bool IsAttributeFull(EItemStats ItemStats, UAbilitySystemComponent *ASC);

    TSubclassOf<AM_Interactive_Item> ItemClassToSpawn;
};