// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "M_BackendComponent.generated.h"

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_BackendComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_BackendComponent();

    void LoadPlayerData();

    UFUNCTION(Server, Reliable)
    void Server_SavePlayerStats();

    // UFUNCTION(Client, Reliable)
    void Client_RequestStatsSave();

    UPROPERTY()
    FString PlayerAuthToken;

    void StartStatsSaveTimer();

    bool bStatsLoaded = false;

    UFUNCTION(Client, Reliable)
    void Client_ReceivePlayerStats(int32 Level, float XP, int32 Gold, float Health, float Mana, int32 Armor,
                                   int32 MagicResistance, FVector LastLocation);

  protected:
    virtual void BeginPlay() override;

  private:
    void OnPlayerStatsSaved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnPlayerDataLoaded(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
                            TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);

    FTimerHandle StatsSaveTimer;
};
