// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "M_InteractionTraceComponent.generated.h"

class UM_ShowNotification;
class AM_Interactive_Item;
class UGameplayEffect;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_InteractionTraceComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_InteractionTraceComponent();

    void TraceForItem();

    void TryInteract();

    UFUNCTION(Server, Reliable)
    void Server_SetPlayerLocation(FVector Location);

    UFUNCTION(Server, Reliable)
    void Server_Interact(AM_Interactive_Item *Item);

    UFUNCTION(Client, Reliable)
    void Client_ShowNotification(const FText &Message);

    UFUNCTION(Client, Reliable)
    void Client_ShowItemName(const FText &ItemName);

    UFUNCTION(Client, Reliable)
    void Client_CantUseItem();

    UFUNCTION(Server, Reliable)
    void Server_ApplyInventoryEffect(bool bOpen);

    UPROPERTY(EditDefaultsOnly, Category = "Interaction")
    TSubclassOf<UM_ShowNotification> NotificationWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Interaction")
    float TraceDistance = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<UGameplayEffect> InventoryOpenEffect;

    UPROPERTY()
    TObjectPtr<UM_ShowNotification> NotificationWidget;

    TWeakObjectPtr<AActor> FocusedItem;

  private:
    TWeakObjectPtr<AActor> LastFocusedItem;

    FActiveGameplayEffectHandle InventoryEffectHandle;
};
