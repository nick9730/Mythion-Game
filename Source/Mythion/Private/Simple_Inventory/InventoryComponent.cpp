// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"  
#include "Simple_Inventory/Data/InventoryData.h"
#include "Characters/PlayerCharacter.h"
#include "M_PlayerController.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);
    Items.SetNum(32);
    

}



void UInventoryComponent::Server_AddWeaponSlot_Implementation(FItemData Item)
{
    if (!Item.IsValid()) return;
    if (Item.ItemType != EItemType::Weapon) return;

    WeaponSlot = Item;
    OnWeaponEquipped.Broadcast(Item);
    OnInventoryChanged.Broadcast();
}
void UInventoryComponent::Server_AddArmorSlot_Implementation(FItemData Item)
{
    if (!Item.IsValid()) return;
    if (Item.ItemType != EItemType::Armor) return;

    ArmorSlot = Item;
    OnWeaponEquipped.Broadcast(Item);
    OnInventoryChanged.Broadcast();
}
void UInventoryComponent::Server_RemoveArmorSlot_Implementation()
{
    if (!ArmorSlot.IsValid()) return;
    ArmorSlot = FItemData();
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryComponent, Items);
    DOREPLIFETIME(UInventoryComponent, WeaponSlot);
    DOREPLIFETIME(UInventoryComponent, ArmorSlot);
}

void UInventoryComponent::Server_AddItem_Implementation(FItemData Item)
{

    for (int32 i = 0; i < Items.Num(); i++)
    {

        if (!Items[i].IsValid()) continue;
        if (Items[i].ItemID != Item.ItemID) continue;
        if (Items[i].Quantity >= Items[i].MaxStackSize) continue;

        int32 SpaceLeft = Items[i].MaxStackSize - Items[i].Quantity;
        int32 ToAdd = FMath::Min(Item.Quantity, SpaceLeft);
        Items[i].Quantity += ToAdd;
        Item.Quantity -= ToAdd;
        OnInventoryChanged.Broadcast();
        if (Item.Quantity <= 0) return;
    }

    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (!Items[i].IsValid())
        {
            Items[i] = Item;
            OnInventoryChanged.Broadcast();
            return;
        }
    }

    /*
    APlayerCharacter* Char = Cast<APlayerCharacter>(GetOwner());
    if (IsValid(Char))
    {
        AM_PlayerController* PC = Cast<AM_PlayerController>(Char->GetController());
        if (IsValid(PC))
            PC->Client_OnInventoryFull();
    }
    */
}

void UInventoryComponent::Server_RemoveItem_Implementation(int32 SlotIndex)
{
    if (!Items.IsValidIndex(SlotIndex)) return;
    Items[SlotIndex] = FItemData();
    OnInventoryChanged.Broadcast();


}
void UInventoryComponent::Server_RemoveWeaponSlot_Implementation()
{
    if (!WeaponSlot.IsValid()) return;
    WeaponSlot = FItemData();
    OnInventoryChanged.Broadcast();
}
void UInventoryComponent::Server_EquipItem_Implementation(FItemData Item)
{

    if (!Item.IsValid()) return;

    if (Item.ItemType == EItemType::Weapon)
    { 
        WeaponSlot = Item;
        OnWeaponEquipped.Broadcast(Item);
        OnInventoryChanged.Broadcast();
    }
    else if (Item.ItemType == EItemType::Armor)
    {
        ArmorSlot = Item;
        OnWeaponEquipped.Broadcast(Item);
        OnInventoryChanged.Broadcast();
    }

    OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::HasSpace(FItemData Item) const
{
 
        for (const FItemData& Slot : Items)
        {
            if (!Slot.IsValid()) return true;
            if (Slot.ItemID == Item.ItemID && Slot.Quantity < Slot.MaxStackSize) return true;
        }
		return false;
}





void UInventoryComponent::OnRep_Inventory()
{
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_WeaponSlot()
{
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_ArmorSlot()
{
    OnInventoryChanged.Broadcast();
}