// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "M_RespawnComponent.generated.h"

class UM_RespawnWidget;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_RespawnComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_RespawnComponent();

    UFUNCTION(Server, Reliable)
    void Server_Respawn();

    UFUNCTION(Client, Reliable)
    void Client_ShowRespawnWidget();

    UFUNCTION(Client, Reliable)
    void Client_HideRespawnWidget();

    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    TSubclassOf<UM_RespawnWidget> RespawnWidgetClass;

    FVector GetCorrectZLocation(FVector TargetLocation);
    FVector GetSafeRespawnPoint(FVector DeathLocation);

  private:
    UPROPERTY()
    TObjectPtr<UM_RespawnWidget> RespawnWidget;
};
