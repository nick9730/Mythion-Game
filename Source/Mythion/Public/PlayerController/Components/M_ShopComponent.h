// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "M_ShopComponent.generated.h"

class AM_MerchantNPC;
class UM_ShopWidget;
class USoundBase;
class UGameplayEffect;
struct FItemData;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_ShopComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_ShopComponent();

    UFUNCTION(Client, Reliable)
    void Client_OpenShop(AM_MerchantNPC *Merchant);

    UFUNCTION(Server, Reliable)
    void Server_BuyItemFromShop(FItemData Item);

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TSubclassOf<UM_ShopWidget> ShopWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TObjectPtr<USoundBase> GreetingSoundShop;

    UPROPERTY(EditDefaultsOnly, Category = "Shop")
    TObjectPtr<USoundBase> GoodByeSoundShop;

    UPROPERTY(EditDefaultsOnly, Category = "Economy")
    TSubclassOf<UGameplayEffect> CoinEffect;

    UPROPERTY()
    TObjectPtr<UM_ShopWidget> ShopWidget;
};
