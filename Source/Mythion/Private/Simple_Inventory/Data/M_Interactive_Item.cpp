// Fill out your copyright notice in the Description page of Project Settings.

#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Widgets/PlayerWidget/M_ShowNotification.h"

#include "M_PlayerController.h"
#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "PlayerController/Components/M_UIComponent.h"

// Sets default values
AM_Interactive_Item::AM_Interactive_Item()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
    InteractSphere->SetupAttachment(RootComponent);
}

void AM_Interactive_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AM_Interactive_Item, ItemData);
    DOREPLIFETIME(AM_Interactive_Item, Quantity);
}

void AM_Interactive_Item::UpdateMesh()
{
    if (IsValid(ItemData) && IsValid(ItemData->StaticMesh))
    {
        Mesh->SetStaticMesh(ItemData->StaticMesh);
    }
}

void AM_Interactive_Item::OnRep_ItemData()
{
    UpdateMesh();
}

void AM_Interactive_Item::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();

    if (IsValid(ItemData) && IsValid(ItemData->StaticMesh))
    {
        Mesh->SetStaticMesh(ItemData->StaticMesh);
    }
    if (IsValid(Mesh))
        OriginalMaterial = Mesh->GetMaterial(0);
}

void AM_Interactive_Item::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));

    TimeElapsed += DeltaTime;
    float NewZ = StartLocation.Z + FMath::Sin(TimeElapsed) * 20.f;
    SetActorLocation(FVector(StartLocation.X, StartLocation.Y, NewZ));
}

void AM_Interactive_Item::Server_Interact_Implementation(AActor *Caller)
{

    Interact(Caller);
}

void AM_Interactive_Item::Interact(AActor *PlayerChar)
{

    if (!HasAuthority())
        return;

    APlayerCharacter *Player = Cast<APlayerCharacter>(PlayerChar);
    if (!IsValid(Player) || !IsValid(ItemData))
        return;

    AM_PlayerController *PC = Cast<AM_PlayerController>(Player->GetController());
    UInventoryComponent *InvComp = Player->InventoryComponent;
    if (!IsValid(InvComp))
        return;

    FItemData NewItem;
    NewItem.ItemID = ItemData->ItemID;
    NewItem.ItemName = ItemData->ItemName;
    NewItem.ItemType = ItemData->ItemType;
    NewItem.MaxStackSize = ItemData->MaxStackSize;
    NewItem.Icon = ItemData->Icon;
    NewItem.Quantity = Quantity;
    NewItem.Socket = ItemData->Socket;
    NewItem.InteractiveClass = ItemData->ItemClassToSpawn;
    NewItem.ItemDataAsset = ItemData;

    if (NewItem.ItemType == EItemType::Weapon && !HasRequiredClassTag(PlayerChar, NewItem))
    {

        if (Player->PlayerClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Classes.Rogue"))))
        {
            if (IsValid(PC->InteractionTraceComponent))

                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("This weapon is for the mage")));
        }
        else
        {
            if (IsValid(PC->InteractionTraceComponent))

                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("This weapon is for the warrior")));
        }
        return;
    }

    if (NewItem.ItemType == EItemType::Armor && !HasRequiredClassTag(PlayerChar, NewItem))
    {

        if (Player->PlayerClassTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Classes.Rogue"))))
        {
            if (IsValid(PC->InteractionTraceComponent))

                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("This Armor is for the mage")));
        }
        else
        {
            if (IsValid(PC->InteractionTraceComponent))

                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("This Armor is for the warrior")));
        }
        return;
    }

    if (NewItem.ItemType == EItemType::Weapon)
    {
        if (!CantEquipWeapon(PlayerChar))
            return;
        InvComp->Server_EquipItem(NewItem);
    }
    else if (NewItem.ItemType == EItemType::Armor)
    {
        if (!CantEquipWeapon(PlayerChar))
            return;
        InvComp->Server_EquipItem(NewItem);
    }
    else
    {

        if (!CheckTheSpaceOfInventory(PlayerChar))
        {

            PC->InteractionTraceComponent->Client_ShowNotification(FText::FromString(TEXT("Inventory is Full")));
            return;
        }
        InvComp->Server_AddItem(NewItem);
    }

    Destroy();
}

// Cant Equip Weapon if Weapon Slot is full
bool AM_Interactive_Item::CantEquipWeapon(AActor *PlayerChar)
{
    APlayerCharacter *Player = Cast<APlayerCharacter>(PlayerChar);
    if (!IsValid(Player) || !IsValid(ItemData))
        return false;

    UInventoryComponent *InvComp = Player->InventoryComponent;
    if (!IsValid(InvComp))
        return false;
    if (ItemData->ItemType == EItemType::Weapon)
    {

        if (InvComp->WeaponSlot.IsValid())
        {
            AM_PlayerController *PC = Cast<AM_PlayerController>(Player->GetController());
            if (IsValid(PC))
                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("You have already a weapon in this slot")));
            return false;
        }
    }
    else if (ItemData->ItemType == EItemType::Armor)
    {

        if (InvComp->ArmorSlot.IsValid())
        {
            AM_PlayerController *PC = Cast<AM_PlayerController>(Player->GetController());
            if (IsValid(PC))
                PC->InteractionTraceComponent->Client_ShowNotification(
                    FText::FromString(TEXT("You have already a armor in this slot")));
            return false;
        }
    }
    return true;
}
// Check if there is space in the inventory for the item
bool AM_Interactive_Item::CheckTheSpaceOfInventory(AActor *PlayerChar)
{

    APlayerCharacter *Player = Cast<APlayerCharacter>(PlayerChar);
    if (!IsValid(Player) || !IsValid(ItemData))
        return false;

    UInventoryComponent *InvComp = Player->InventoryComponent;
    if (!IsValid(InvComp))
        return false;

    for (const FItemData &Slot : InvComp->Items)
    {
        if (!Slot.IsValid())
            return true;
        if (Slot.ItemID == ItemData->ItemID && Slot.Quantity < Slot.MaxStackSize)
            return true;
    }

    return false;
}

// Has Required Tag
bool AM_Interactive_Item::HasRequiredClassTag(AActor *PlayerChar, FItemData Item)
{

    APlayerCharacter *Player = Cast<APlayerCharacter>(PlayerChar);
    if (!IsValid(Player) || !IsValid(ItemData))
        return false;
    if (!IsValid(Item.ItemDataAsset))
        return false;

    return Item.ItemDataAsset->ItemTag.HasTag(Player->PlayerClassTag);
}

void AM_Interactive_Item::OnFocused()
{

    if (IsValid(Mesh) && IsValid(HighlightMaterial))
        Mesh->SetMaterial(0, HighlightMaterial);
}

void AM_Interactive_Item::OnUnfocused()
{

    if (IsValid(Mesh) && IsValid(OriginalMaterial))
        Mesh->SetMaterial(0, OriginalMaterial);

    AM_PlayerController *PC = Cast<AM_PlayerController>(GetOwner());
    if (IsValid(PC) && IsValid(PC->InteractionTraceComponent->NotificationWidget))
    {
        PC->InteractionTraceComponent->NotificationWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

AM_Interactive_Item *AM_Interactive_Item::SpawnItem(UWorld *World, UM_Item_Details *InItemData, int32 InQuantity,
                                                    FVector Location, TSubclassOf<AM_Interactive_Item> ItemClass)
{
    if (!IsValid(World) || !IsValid(InItemData) || !IsValid(ItemClass))
        return nullptr;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AM_Interactive_Item *SpawnedItem =
        World->SpawnActor<AM_Interactive_Item>(ItemClass, Location, FRotator::ZeroRotator, SpawnParams);

    if (IsValid(SpawnedItem))
    {
        SpawnedItem->ItemData = InItemData;
        SpawnedItem->Quantity = InQuantity;
        SpawnedItem->StartLocation = Location;
        SpawnedItem->UpdateMesh();
        SpawnedItem->ForceNetUpdate();
    }

    return SpawnedItem;
}
