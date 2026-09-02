// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "AM_QuestNPC.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Enemies/M_Enemy_Area_Spawner.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "M_PlayerController.h"
#include "Shop/M_MerchantNPC.h"
#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Widgets/PlayerWidget/M_ShowNotification.h"

// Sets default values for this component's properties
UM_InteractionTraceComponent::UM_InteractionTraceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UM_InteractionTraceComponent::Client_ShowNotification_Implementation(const FText &Message)
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (!IsValid(NotificationWidget))
    {
        NotificationWidget = CreateWidget<UM_ShowNotification>(OwnerPC, NotificationWidgetClass);
        if (!IsValid(NotificationWidget))
            return;
        NotificationWidget->AddToViewport();
    }
    NotificationWidget->ShowMessage(Message);
}

void UM_InteractionTraceComponent::Client_ShowItemName_Implementation(const FText &ItemName)
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (!IsValid(NotificationWidget))
    {
        NotificationWidget = CreateWidget<UM_ShowNotification>(OwnerPC, NotificationWidgetClass);
        if (!IsValid(NotificationWidget))
            return;
        NotificationWidget->AddToViewport();
    }
    NotificationWidget->ShowNameOfItem(ItemName);
}

void UM_InteractionTraceComponent::Client_CantUseItem_Implementation()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("You can't use this item!"));
}

void UM_InteractionTraceComponent::Server_ApplyInventoryEffect_Implementation(bool bOpen)
{
    APlayerController *OwnerController = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PC))
        return;

    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    if (!IsValid(PS))
        return;

    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    if (bOpen)
    {
        ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Inventory.Open")));

        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(InventoryOpenEffect, 1, Context);
        InventoryEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
    else
    {
        ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Inventory.Open")));
        ASC->RemoveActiveGameplayEffect(InventoryEffectHandle);
    }
}

void UM_InteractionTraceComponent::TraceForItem()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController) || !OwnerController->IsLocalController())
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PlayerChar))
        return;

    FVector TraceStart = PlayerChar->GetFollowCamera()->GetComponentLocation();
    FVector TraceEnd = TraceStart + PlayerChar->GetFollowCamera()->GetForwardVector() * TraceDistance;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(PlayerChar);

    GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    LastFocusedItem = FocusedItem;
    FocusedItem = HitResult.GetActor();

    if (FocusedItem == LastFocusedItem)
        return;

    if (LastFocusedItem.IsValid())
    {
        AM_Interactive_Item *OldItem = Cast<AM_Interactive_Item>(LastFocusedItem.Get());
        if (IsValid(OldItem))
            OldItem->OnUnfocused();

        AAM_QuestNPC *NewNPC = Cast<AAM_QuestNPC>(LastFocusedItem.Get());
        if (IsValid(NewNPC))
            NewNPC->OnUnfocused();

        AM_MerchantNPC *NewMerchant = Cast<AM_MerchantNPC>(LastFocusedItem.Get());
        if (IsValid(NewMerchant))
            NewMerchant->OnUnfocused();

        if (IsValid(NotificationWidget) && !NotificationWidget->bIsShowingSystemMessage)
        {
            NotificationWidget->HideNotification();
        }
    }

    if (FocusedItem.IsValid())
    {
        AM_Interactive_Item *NewItem = Cast<AM_Interactive_Item>(FocusedItem.Get());
        if (IsValid(NewItem))
        {
            NewItem->OnFocused();
            if (IsValid(NewItem->ItemData))
                Client_ShowItemName(FText::FromString(NewItem->ItemData->ItemName.ToString()));
        }
        else
        {
            if (IsValid(NotificationWidget))
                NotificationWidget->SetVisibility(ESlateVisibility::Hidden);
        }

        AAM_QuestNPC *NewNPC = Cast<AAM_QuestNPC>(FocusedItem.Get());
        if (IsValid(NewNPC))
            NewNPC->OnFocused();

        AM_MerchantNPC *NewMerchant = Cast<AM_MerchantNPC>(FocusedItem.Get());
        if (IsValid(NewMerchant))
            NewMerchant->OnFocused();
    }
}

void UM_InteractionTraceComponent::Server_Interact_Implementation(AM_Interactive_Item *Item)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    if (!IsValid(Item))
        return;

    Item->Interact(OwnerController->GetPawn());
}

void UM_InteractionTraceComponent::Server_SetPlayerLocation_Implementation(FVector Location)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *Char = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(Char))
        return;

    TArray<AActor *> Spawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AM_Enemy_Area_Spawner::StaticClass(), Spawners);

    FVector FinalLocation = Location;

    for (AActor *Spawner : Spawners)
    {
        AM_Enemy_Area_Spawner *SpawnAreaD = Cast<AM_Enemy_Area_Spawner>(Spawner);
        if (!IsValid(SpawnAreaD))
            continue;

        FVector BoxOrigin = SpawnAreaD->SpawnArea->GetComponentLocation();
        FVector BoxExtent = SpawnAreaD->SpawnArea->GetScaledBoxExtent();

        if (SpawnAreaD->IsLocationInsideBox(Location))
        {
            FinalLocation = SpawnAreaD->GetSafeSpawnLocation(Location);
            break;
        }
    }

    Char->SetActorLocation(FinalLocation);
}

void UM_InteractionTraceComponent::TryInteract()
{
    if (!FocusedItem.IsValid())
        return;

    AM_Interactive_Item *Item = Cast<AM_Interactive_Item>(FocusedItem.Get());
    if (IsValid(Item))
    {
        Server_Interact(Item);
    }

    AAM_QuestNPC *NPC = Cast<AAM_QuestNPC>(FocusedItem.Get());
    if (IsValid(NPC))
    {
        NPC->Interact(GetOwner() ? Cast<AController>(GetOwner())->GetPawn() : nullptr);
    }

    AM_MerchantNPC *Merchant = Cast<AM_MerchantNPC>(FocusedItem.Get());
    if (IsValid(Merchant))
    {
        AController *OwnerController = Cast<AController>(GetOwner());
        if (IsValid(OwnerController))
            Merchant->Interact(OwnerController->GetPawn());
    }
}