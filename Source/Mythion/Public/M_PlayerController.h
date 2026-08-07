// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Backend/M_BackendSubsystem.h"
#include "CoreMinimal.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "GameplayEffect.h"
#include "InputActionValue.h"
#include "Shop/UI/M_ShopWidget.h"
#include "Simple_Inventory/Widgets/M_QuantityWidget.h"
#include "Widgets/PlayerWidget/M_RespawnWidget.h"

#include "M_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UStatsWidget;
class UM_AbilitiesContainer;
class UM_Inventory;
class UInv_HUDWidget;
class AAM_QuestNPC;
class UM_QuestDialolgWidget;
class UM_QuestDataAsset;
struct FQuestData;
class UM_QuestsInfos;
class UM_CompletedQuestWidget;

class IHttpRequest;
class IHttpResponse;

using FHttpRequestPtr = TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>;
using FHttpResponsePtr = TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>;

class FText;

class UM_ShowNotification;
class UM_RespawnWidget;
class AM_MerchantNPC;
class UM_ShopWidget;

class USoundBase;
class UAudioComponent;

UCLASS()
class MYTHION_API AM_PlayerController : public APlayerController
{
    GENERATED_BODY()

  public:
    // Sounds
    UPROPERTY(EditDefaultsOnly, Category = "Game Sound")
    TArray<USoundBase *> GenericGameSounds;

    UPROPERTY()
    int32 SoundIndex = 0;

    UFUNCTION()
    void PlayTheSound();

    UPROPERTY()
    UAudioComponent *GenericSoundAudioComponent;

    UFUNCTION()
    void OnFinishingGenericSound();

    UPROPERTY(EditDefaultsOnly, Category = "ShopSound")
    USoundBase *GreetingSoundShop;

    UPROPERTY(EditDefaultsOnly, Category = "ShopSound ")
    USoundBase *GoodByeSoundShop;

    UPROPERTY(EditDefaultsOnly, Category = "CombatSound")
    TArray<USoundBase *> SoundCombat;

    UFUNCTION(Client, Reliable)
    void Client_NotifyUserByEnemyPerception(bool bPerceived, AActor *Enemy);

    UPROPERTY(EditDefaultsOnly, Category = "Music")
    float MusicDebounceDelay = 3.0f;

    FTimerHandle MusicDebounceTimer;

    UPROPERTY()
    TArray<AActor *> EnemiesSpottingMe;

    bool GetSpotted() const
    {
        return EnemiesSpottingMe.Num() > 0;
    }

    // Other things

    AM_PlayerController();

    UFUNCTION(BlueprintCallable)
    void ShowLoadingScreen();
    void HideLoadingScreen();

    // Backend

    // Register
    UFUNCTION(Client, Reliable)
    void Client_SetTokenAndTravel(const FString &Token, const FString &ServerIP);

    // Stats
    void LoadPlayerData();
    void LoadPlayerForRespawn();

    // Inventory
    // void SaveInventory();
    void LoadInventory();

    UFUNCTION()
    FVector GetCorrectZLocation(FVector TargetLocation);

    UPROPERTY()
    FString PlayerAuthToken;

    UFUNCTION(Server, Reliable, BlueprintCallable)
    void Server_SetAuthToken(const FString &Token);

    UFUNCTION(Server, Reliable)
    void Server_SetupAttributes(int32 Level, int32 XP, float Mana, float Health, float Coins);

    UFUNCTION(Server, Reliable)
    void Server_SavePlayerStats();

    UFUNCTION(Client, Reliable)
    void Client_ReceivePlayerStats(int32 Level, float XP, int32 Gold, float Health, float Mana, int32 Armor,
                                   int32 MagicResistance, FVector LastLocation);

    // Quests
    UFUNCTION(Client, Reliable)
    void Client_ShowQuestDialog(AAM_QuestNPC *NPC, FQuestData QuestData);

    // Status
    UFUNCTION(Client, Reliable)
    void Client_PendingQuestLoaded(FQuestData QuestAsset);

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UM_QuestDialolgWidget> QuestDialogClass;

    UPROPERTY()
    UM_QuestDialolgWidget *QuestDialogWidget;

    // Update and Complete Widgets Quests

    void UpdateNPCQuestStatus(FQuestData Quest, EQuestStatus Status);

    UFUNCTION(Server, Reliable)
    void Server_CompleteQuest(FQuestData QuestAsset);

    UFUNCTION(Client, Reliable)
    void Client_CompleteQuest(FQuestData QuestAsset);

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UM_QuestsInfos> QuestInfoClass;

    UPROPERTY()
    UM_QuestsInfos *QuestInfoWidget;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UM_CompletedQuestWidget> QuestCompletedClass;

    UPROPERTY()
    UM_CompletedQuestWidget *QuestCompletedWidget;

    UFUNCTION()
    void ShowCompletedQuest(FQuestData Quest);

    // Quests and Backend
    void SavePlayerQuestsForBackEnd(FQuestData Quest);

    void LoadPlayerQuests();

    UFUNCTION(Client, Reliable)
    void Client_UpdateKills(FQuestData Quest, int32 CurrentKills);

    // Quests

    UFUNCTION(Server, Reliable)
    void Server_RequestInventoryData();
    UFUNCTION(Client, Reliable)
    void Client_ReceiveInventoryData(const TArray<FItemData> &ServerItems, FItemData WeaponSlot, FItemData ArmorSlot);

    void Client_RequestInventorySave();

    void Client_RequestStatsSave();

    // Inventory
    FTimerHandle InventoryInitTimer;
    void TryInitInventory();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UpdateInventoryAfterLoad(const TArray<FItemData> &LoadedItems, FItemData WeaponItem,
                                         FItemData ArmorItem);

    UFUNCTION(Server, Reliable)
    void Server_DropItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_SellItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_SellWeapon();

    UFUNCTION(Server, Reliable)
    void Server_SellArmor();

    UFUNCTION(Server, Reliable)
    void Server_UseItem(int32 SlotIndex);

    bool IsAttributeFull(EItemStats ItemStats, UAbilitySystemComponent *ASC);

    UFUNCTION(Server, Reliable)
    void Server_DropWeaponSlot();

    UFUNCTION(Server, Reliable)
    void Server_UseWeaponSlot();

    UFUNCTION(Server, Reliable)
    void Server_DropArmorSlot();

    UFUNCTION(Server, Reliable)
    void Server_UseArmorsSlot();

    UFUNCTION(Client, Reliable)
    void Client_OpenQuantityWidget(FItemData ItemData, FVector2D SlotPosition, int32 SlotIndex);

    UPROPERTY()
    UM_QuantityWidget *QuantityWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UM_QuantityWidget> QuantityWidgetClass;

    // Fu
    UFUNCTION(Client, Reliable)
    void Client_ShowNotification(const FText &Message);

    UFUNCTION(Client, Reliable)
    void Client_ShowItemName(const FText &ItemName);

    UPROPERTY()
    UM_ShowNotification *NotificationWidget;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UM_ShowNotification> NotificationWidgetClass;

    UFUNCTION(Client, Reliable)
    void Client_CantUseItem();

    UFUNCTION(Server, Reliable)
    void Server_SetPlayerLocation(FVector Location);

    UFUNCTION(Server, Reliable)
    void Server_Respawn();

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UM_RespawnWidget> RespawnWidgetClass;

    UPROPERTY()
    UM_RespawnWidget *RespawnWidget;

    UFUNCTION(Client, Reliable)
    void Client_ShowRespawnWidget();

    UFUNCTION(Client, Reliable)
    void Client_HideRespawnWidget();

    // Options
    UPROPERTY(EditDefaultsOnly, Category = "Widgets")
    TSubclassOf<UUserWidget> OptionsWidgetClass;

    UPROPERTY()
    UUserWidget *OptionsWidgetInstance;

    UFUNCTION(Client, Reliable, BlueprintCallable)
    void Client_ShowOptionsWidget();

    UFUNCTION(Client, Reliable, BlueprintCallable)
    void Client_HideOptionsWidget();

    // Quests
    // void ShowQuestDialog(AAM_QuestNPC* NPC, FQuestData QuestData);
    // void ShowQuestInfos(FQuestData QuestData);
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UM_QuestDialolgWidget> QuestWidgetDialogClass;

    UPROPERTY()
    UM_QuestDialolgWidget *QuestWidgetDialog;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UM_QuestsInfos> QuestInfoWidgetClass;

    void TryInteract();

    UPROPERTY()
    TSubclassOf<AM_Interactive_Item> ItemClassToSpawn;

    bool bIsHostPlayer = false;

    // Shop
    UFUNCTION(Client, Reliable)
    void Client_OpenShop(AM_MerchantNPC *Merchant);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    TSubclassOf<UM_ShopWidget> ShopWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
    UM_ShopWidget *ShopWidget;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> CoinEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> CoinEffectSell;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> TakeReward;

    UFUNCTION(Server, Reliable)
    void Server_BuyItemFromShop(FItemData Item);

    // Chat
    UFUNCTION()
    void HandleChatConnectedTest();

    // Flags for filling out all the  thing from the backend
    bool bStatsLoaded = false;
    bool bInventoryLoaded = false;
    bool bQuestsLoaded = false;
    bool bOnASCReadyForAbilities = false;

    void CheckAllLoaded();

    UFUNCTION()
    FVector GetSafeRespawnPoint(FVector DeathLocation);

  protected:
    // Initialization
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn *InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    bool bRespawned = false;

    UFUNCTION()
    void OnASCReady(UAbilitySystemComponent *ASC, UAttributeSet *AS);

    UPROPERTY(EditAnywhere)
    TSubclassOf<UM_AbilitiesContainer> AbilityBarClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UStatsWidget> StatsWidgetClass;

    UPROPERTY(BlueprintReadOnly)
    UStatsWidget *StatsWidget;

    // Input Actions
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

    // For now nothing
    UPROPERTY(EditAnywhere, Category = "Combat")
    TObjectPtr<UAnimMontage> AttackMontage;
    //

    // Inventory
    UPROPERTY(EditAnywhere)
    TSubclassOf<UM_Inventory> InventoryWidgetClass;

    UPROPERTY()
    UM_Inventory *InventoryWidget;

    UFUNCTION(BlueprintCallable)
    void ToggleInventory();

    UPROPERTY()
    bool bInventoryOpen = false;

    UFUNCTION(Server, Reliable)
    void Server_Interact(AM_Interactive_Item *Item);
    UPROPERTY(EditAnywhere, Category = "Inventory")
    TSubclassOf<UGameplayEffect> InventoryOpenEffect;

    FActiveGameplayEffectHandle InventoryEffectHandle;

    UFUNCTION(Server, Reliable)
    void Server_ApplyInventoryEffect(bool bOpen);

    UFUNCTION()
    void OnInventoryFull();

    UPROPERTY(EditDefaultsOnly, Category = "Mythion | Inventory")
    double TraceDistance;

    TWeakObjectPtr<AActor> FocusedItem;
    TWeakObjectPtr<AActor> LastFocusedItem;

    void TraceForItem();

  private:
    UPROPERTY(EditDefaultsOnly, Category = "Mythion | Contexts")
    TArray<TObjectPtr<UInputMappingContext>> AddedMappingContexts;

    UPROPERTY(EditDefaultsOnly, Category = "Mythion | Sounds")
    USoundBase *Sound;

    UPROPERTY(EditDefaultsOnly, Category = "Mythion | Sounds")
    USoundBase *CloseSound;
    // Interactions
    void Move(const FInputActionValue &Value);
    void Look(const FInputActionValue &Value);
    void Jump();
    void StopJumping();

  protected:
    // Backend
    void OnPlayerDataLoaded(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
                            TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);
    void OnPlayerDataForRespawn(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
                                TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);
    void OnPlayerStatsSaved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    void OnInventorySaved(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                          TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);
    void OnInventoryLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                           TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);

    void OnPlayerQuestsLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                              TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful);

    FTimerHandle AutoSaveTimer;
    FTimerHandle InventorySaveTimer;
    FTimerHandle StatsSaveTimer;
};
