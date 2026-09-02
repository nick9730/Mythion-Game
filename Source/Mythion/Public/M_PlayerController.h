// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"

#include "M_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UStatsWidget;
class UM_AbilitiesContainer;

class UM_MusicComponent;
class UM_BackendComponent;
class UM_InventoryActionsComponent;
class UM_InteractionTraceComponent;
class UM_RespawnComponent;
class UM_QuestUIComponent;
class UM_ShopComponent;
class UM_UIComponent;

UCLASS()
class MYTHION_API AM_PlayerController : public APlayerController
{
    GENERATED_BODY()

  public:
    AM_PlayerController();

    //  Components
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_MusicComponent> MusicComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_BackendComponent> BackendComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_InventoryActionsComponent> InventoryActionsComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_RespawnComponent> RespawnComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_QuestUIComponent> QuestUIComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_InteractionTraceComponent> InteractionTraceComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_ShopComponent> ShopComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UM_UIComponent> UIComponent;

    //  Widgets not yet moved
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UStatsWidget> StatsWidget;

    //  Backend auth
    UPROPERTY()
    FString PlayerAuthToken;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetAuthToken(const FString &Token);

    UFUNCTION(Server, Reliable)
    void Server_SetupAttributes(int32 Level, int32 XP, float Mana, float Health, float Coins, int32 Armor,
                                int32 MagicResist);

    //  Load-state tracking
    bool bStatsLoaded = false;
    bool bInventoryLoaded = false;
    bool bQuestsLoaded = false;
    bool bOnASCReadyForAbilities = false;
    void CheckAllLoaded();

    UFUNCTION(BlueprintCallable)
    void ShowLoadingScreen();
    void HideLoadingScreen();

    UFUNCTION(Client, Reliable)
    void Client_NotifyUserByEnemyPerception(bool bPerceived, AActor *Enemy);

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn *InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION()
    void OnASCReady(UAbilitySystemComponent *ASC, UAttributeSet *AS);

    UFUNCTION()
    void HandleChatConnectedTest();

    UPROPERTY(EditAnywhere)
    TSubclassOf<UM_AbilitiesContainer> AbilityBarClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UStatsWidget> StatsWidgetClass;

    //  Input
    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InventoryAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

  private:
    UPROPERTY(EditDefaultsOnly, Category = "Mythion | Contexts")
    TArray<TObjectPtr<UInputMappingContext>> AddedMappingContexts;

    void Move(const FInputActionValue &Value);
    void Look(const FInputActionValue &Value);
    void Jump();
    void StopJumping();
};