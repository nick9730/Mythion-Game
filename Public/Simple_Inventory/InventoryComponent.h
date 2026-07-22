// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryFull);

class UM_Item_Details;
class AM_PlayerController;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTHION_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

public:
    UInventoryComponent();

    UPROPERTY(ReplicatedUsing = OnRep_Inventory, BlueprintReadOnly)
   TArray<FItemData> Items;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponSlot, BlueprintReadOnly)
    FItemData WeaponSlot;

    UPROPERTY(ReplicatedUsing = OnRep_ArmorSlot, BlueprintReadOnly)
   FItemData ArmorSlot;
    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;

    UFUNCTION(Server, Reliable)
    void Server_AddItem(FItemData Item);

    UFUNCTION(Server, Reliable)
    void Server_RemoveItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_EquipItem(int32 SlotIndex);

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryFull;

    UPROPERTY()
    AM_PlayerController* OwnerController;

private:
    UFUNCTION()
    void OnRep_Inventory();

    UFUNCTION()
    void OnRep_WeaponSlot();

    UFUNCTION()
    void OnRep_ArmorSlot();

    static const int32 MaxSlots = 16;
};

		

