
#include "PlayerController/Components/M_InventoryActionsComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Simple_Inventory/InventoryComponent.h"

UM_InventoryActionsComponent::UM_InventoryActionsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UM_InventoryActionsComponent::Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData DroppedItem = PlayerChar->InventoryComponent->Items[SlotIndex];
    if (!DroppedItem.IsValid())
        return;

    ItemClassToSpawn = DroppedItem.InteractiveClass;
    int32 DropQuantity = FMath::Clamp(Quantity, 1, DroppedItem.Quantity);

    if (DropQuantity >= DroppedItem.Quantity)
        PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
    else
    {
        PlayerChar->InventoryComponent->Items[SlotIndex].Quantity -= DropQuantity;
        PlayerChar->InventoryComponent->OnInventoryChanged.Broadcast();
    }

    for (int32 i = 0; i < DropQuantity; i++)
    {
        FVector DropLocation = PlayerChar->GetActorLocation() + PlayerChar->GetActorForwardVector() * 100.f +
                               FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 0.f);

        AM_Interactive_Item::SpawnItem(GetWorld(), DroppedItem.ItemDataAsset, 1, DropLocation, ItemClassToSpawn);
    }
}

void UM_InventoryActionsComponent::Server_DropWeaponSlot_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData WeaponItem = PlayerChar->InventoryComponent->WeaponSlot;
    if (!WeaponItem.IsValid())
        return;

    ItemClassToSpawn = WeaponItem.InteractiveClass;
    if (IsValid(ItemClassToSpawn))
    {
        UE_LOG(LogTemp, Display, TEXT("[DROP SYSTEM] Spawning Class: %s"), *ItemClassToSpawn->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[DROP SYSTEM] CRITICAL: ItemClassToSpawn is NULL! Check the DataAsset!"));
    }

    PlayerChar->UnequipWeapon(WeaponItem);
    PlayerChar->InventoryComponent->Server_RemoveWeaponSlot();

    FVector DropLocation = PlayerChar->GetActorLocation() + PlayerChar->GetActorForwardVector() * 100.f +
                           FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 0.f);

    AM_Interactive_Item::SpawnItem(GetWorld(), WeaponItem.ItemDataAsset, 1, DropLocation, ItemClassToSpawn);
}

void UM_InventoryActionsComponent::Server_DropArmorSlot_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData ArmorItem = PlayerChar->InventoryComponent->ArmorSlot;
    if (!ArmorItem.IsValid())
        return;

    PlayerChar->UnequipWeapon(ArmorItem);
    PlayerChar->InventoryComponent->Server_RemoveArmorSlot();

    ItemClassToSpawn = ArmorItem.InteractiveClass;
    FVector DropLocation = PlayerChar->GetActorLocation() + PlayerChar->GetActorForwardVector() * 100.f +
                           FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 0.f);

    AM_Interactive_Item::SpawnItem(GetWorld(), ArmorItem.ItemDataAsset, 1, DropLocation, ItemClassToSpawn);
}

void UM_InventoryActionsComponent::Server_SellItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FItemData SoldItem = PlayerChar->InventoryComponent->Items[SlotIndex];
    if (!SoldItem.IsValid() || !IsValid(SoldItem.ItemDataAsset))
        return;

    int32 SellQuantity = FMath::Clamp(Quantity, 1, SoldItem.Quantity);

    if (SellQuantity >= SoldItem.Quantity)
        PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
    else
    {
        PlayerChar->InventoryComponent->Items[SlotIndex].Quantity -= SellQuantity;
        PlayerChar->InventoryComponent->OnInventoryChanged.Broadcast();
    }

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), SoldItem.ItemDataAsset->SellPrice * SellQuantity);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UM_InventoryActionsComponent::Server_SellWeapon_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FItemData WeaponItem = PlayerChar->InventoryComponent->WeaponSlot;
    if (!WeaponItem.IsValid() || !IsValid(WeaponItem.ItemDataAsset))
        return;

    PlayerChar->InventoryComponent->Server_RemoveWeaponSlot();
    PlayerChar->UnequipWeapon(WeaponItem);

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), WeaponItem.ItemDataAsset->SellPrice);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UM_InventoryActionsComponent::Server_SellArmor_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FItemData ArmorItem = PlayerChar->InventoryComponent->ArmorSlot;
    if (!ArmorItem.IsValid() || !IsValid(ArmorItem.ItemDataAsset))
        return;

    PlayerChar->UnequipWeapon(ArmorItem);
    PlayerChar->InventoryComponent->Server_RemoveArmorSlot();

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), ArmorItem.ItemDataAsset->SellPrice);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

bool UM_InventoryActionsComponent::IsAttributeFull(EItemStats ItemStats, UAbilitySystemComponent *ASC)
{
    switch (ItemStats)
    {
    case EItemStats::HealthRefill:
        return ASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute()) >=
               ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());

    case EItemStats::ManaRefill:
        return ASC->GetNumericAttribute(UM_AttributeSet::GetManaAttribute()) >=
               ASC->GetNumericAttribute(UM_AttributeSet::GetMaxManaAttribute());
    default:
        return false;
    }
}

void UM_InventoryActionsComponent::Server_UseItem_Implementation(int32 SlotIndex)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData &Item = PlayerChar->InventoryComponent->Items[SlotIndex];
    if (!Item.IsValid() || !IsValid(Item.ItemDataAsset))
        return;
    if (!IsValid(Item.ItemDataAsset->UseEffect))
        return;

    UAbilitySystemComponent *AbilitySC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(AbilitySC))
        return;

    bool bIsAttributeFull = IsAttributeFull(Item.ItemDataAsset->ItemStats, AbilitySC);

    if (bIsAttributeFull)
    {
        AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);
        if (IsValid(PCController))
        {
            PCController->InteractionTraceComponent->Client_CantUseItem();
        }
        return;
    }

    FItemData &ItemSlot = PlayerChar->InventoryComponent->Items[SlotIndex];
    if (!ItemSlot.IsValid() || !IsValid(ItemSlot.ItemDataAsset))
        return;
    if (!IsValid(ItemSlot.ItemDataAsset->UseEffect))
        return;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemSlot.ItemDataAsset->UseEffect, 1, Context);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

    ItemSlot.Quantity--;
    if (ItemSlot.Quantity <= 0)
        PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
    else
        PlayerChar->InventoryComponent->OnInventoryChanged.Broadcast();
}

void UM_InventoryActionsComponent::Server_UseWeaponSlot_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData &Item = PlayerChar->InventoryComponent->WeaponSlot;
    if (!Item.IsValid() || !IsValid(Item.ItemDataAsset))
        return;
    if (!IsValid(Item.ItemDataAsset->UseEffect))
        return;

    UAbilitySystemComponent *AbilitySC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(AbilitySC))
        return;

    if (Item.ItemDataAsset->ItemStats == EItemStats::ArmorBuff)
    {
        FItemData &ItemSlot = PlayerChar->InventoryComponent->WeaponSlot;
        if (!ItemSlot.IsValid() || !IsValid(ItemSlot.ItemDataAsset))
            return;
        if (!IsValid(ItemSlot.ItemDataAsset->UseEffect))
            return;

        UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
        if (!IsValid(ASC))
            return;

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemSlot.ItemDataAsset->UseEffect, 1, Context);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}

void UM_InventoryActionsComponent::Server_UseArmorsSlot_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    FItemData &Item = PlayerChar->InventoryComponent->ArmorSlot;
    if (!Item.IsValid() || !IsValid(Item.ItemDataAsset))
        return;
    if (!IsValid(Item.ItemDataAsset->UseEffect))
        return;

    UAbilitySystemComponent *AbilitySC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(AbilitySC))
        return;

    if (Item.ItemDataAsset->ItemStats == EItemStats::ArmorBuff)
    {
        FItemData &ItemSlot = PlayerChar->InventoryComponent->ArmorSlot;
        if (!ItemSlot.IsValid() || !IsValid(ItemSlot.ItemDataAsset))
            return;
        if (!IsValid(ItemSlot.ItemDataAsset->UseEffect))
            return;

        UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
        if (!IsValid(ASC))
            return;

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemSlot.ItemDataAsset->UseEffect, 1, Context);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}

bool UM_InventoryActionsComponent::Server_UpdateInventoryAfterLoad_Validate(const TArray<FItemData> &LoadedItems,
                                                                            FItemData WeaponItem, FItemData ArmorItem)
{
    return true;
}

void UM_InventoryActionsComponent::Server_UpdateInventoryAfterLoad_Implementation(const TArray<FItemData> &LoadedItems,
                                                                                  FItemData WeaponItem,
                                                                                  FItemData ArmorItem)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (IsValid(PC) && IsValid(PC->InventoryComponent))
    {
        PC->InventoryComponent->Items = LoadedItems;
        PC->InventoryComponent->WeaponSlot = WeaponItem;
        PC->OnWeaponEquipped(WeaponItem);

        PC->InventoryComponent->ArmorSlot = ArmorItem;
        PC->OnWeaponEquipped(ArmorItem);
        PC->InventoryComponent->OnInventoryChanged.Broadcast();

        PC->ForceNetUpdate();
    }
}