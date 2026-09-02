// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_BackendComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Attributes/M_AttributeSet.h"
#include "Backend/M_BackendSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "DataAsset/CharacterClasses.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "Dom/JsonObject.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Controller.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "M_PlayerController.h"
#include "M_QuestComponent.h"
#include "PlayerController/Components/M_QuestUIComponent.h"
#include "PlayerController/Components/M_RespawnComponent.h"

#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "PlayerController/Components/M_InventoryActionsComponent.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Simple_Inventory/InventoryComponent.h"

// Sets default values for this component's properties
UM_BackendComponent::UM_BackendComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    // ...
}

void UM_BackendComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UM_BackendComponent::StartStatsSaveTimer()
{
    if (!IsValid(GetOwner()))
        return;

    float RandomStatsDelay = FMath::FRandRange(1.f, 30.f);

    GetOwner()->GetWorldTimerManager().SetTimer(StatsSaveTimer, this, &UM_BackendComponent::Client_RequestStatsSave,
                                                40.f, true, RandomStatsDelay);
}

void UM_BackendComponent::LoadPlayerData()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    UM_BackendSubsystem *BackendSubsystem = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();

    if (!BackendSubsystem || !BackendSubsystem->bIsLoggedIn)
    {
        UE_LOG(LogTemp, Error, TEXT("Not logged in! Cannot load stats."));
        return;
    }

    if (PlayerAuthToken.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Token is empty!"));
        return;
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(BackendSubsystem->ServerURL + TEXT("/api/player/stats"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *PlayerAuthToken));

    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendComponent::OnPlayerDataLoaded);
    Request->ProcessRequest();
}

void UM_BackendComponent::Server_SavePlayerStats_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *Char = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(Char))
        return;

    UAbilitySystemComponent *AbilitySC = Char->GetAbilitySystemComponent();
    if (!IsValid(AbilitySC))
        return;

    FVector Lastlocation = Char->GetActorLocation();

    float CurrentHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    int32 CurrentLevel = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
    float CurrentMana = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetManaAttribute());
    int32 CurrentCoins = FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute()));
    int32 CurrentArmor = FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute()));
    int32 CurrentMagicResist =
        FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute()));
    float CurrentXP = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());

    if (CurrentHealth <= 0.f)
        return;

    Client_ReceivePlayerStats(CurrentLevel, CurrentXP, CurrentCoins, CurrentHealth, CurrentMana, CurrentArmor,
                              CurrentMagicResist, Lastlocation);
}

void UM_BackendComponent::Client_ReceivePlayerStats_Implementation(int32 Level, float XP, int32 Gold, float Health,
                                                                   float Mana, int32 Armor, int32 MagicResistance,
                                                                   FVector LastLocation)
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        return;

    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    UM_BackendSubsystem *Backend = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!Backend || Backend->AuthToken.IsEmpty())
        return;

    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();

    Root->SetNumberField(TEXT("level"), Level);
    Root->SetNumberField(TEXT("xp"), XP);
    Root->SetNumberField(TEXT("coins"), Gold);
    Root->SetNumberField(TEXT("health"), Health);
    Root->SetNumberField(TEXT("mana"), Mana);
    Root->SetNumberField(TEXT("armor"), Armor);
    Root->SetNumberField(TEXT("magicResist"), MagicResistance);

    TSharedPtr<FJsonObject> LocationObj = MakeShared<FJsonObject>();
    LocationObj->SetNumberField(TEXT("x"), LastLocation.X);
    LocationObj->SetNumberField(TEXT("y"), LastLocation.Y);
    LocationObj->SetNumberField(TEXT("z"), LastLocation.Z);
    Root->SetObjectField(TEXT("lastLocation"), LocationObj);

    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/stats"));
    Request->SetVerb(TEXT("PUT"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendComponent::OnPlayerStatsSaved);
    Request->ProcessRequest();
}

void UM_BackendComponent::OnPlayerStatsSaved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save stats to server"));
        return;
    }
    if (Response->GetResponseCode() == 200)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stats saved successfully!"));
    }
}

void UM_BackendComponent::Client_RequestStatsSave()
{
    if (!IsValid(this) || !IsValid(GetOwner()))
        return;

    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        return;

    Server_SavePlayerStats();
}

void UM_BackendComponent::OnPlayerDataLoaded(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
                                             TSharedPtr<class IHttpResponse, ESPMode::ThreadSafe> Response,
                                             bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load stats from server"));
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse stats response"));
        return;
    }

    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    if (Response->GetResponseCode() == 200)
    {
        TSharedPtr<FJsonObject> StatsObject = JsonObject->GetObjectField(TEXT("stats"));

        FString ClassName;
        FString Gender;
        StatsObject->TryGetStringField(TEXT("className"), ClassName);
        StatsObject->TryGetStringField(TEXT("gender"), Gender);

        int32 Level = StatsObject->GetIntegerField(TEXT("level"));
        int32 XP = StatsObject->GetIntegerField(TEXT("xp"));
        float Health = StatsObject->GetNumberField(TEXT("health"));
        float Mana = StatsObject->GetNumberField(TEXT("mana"));
        int32 Coins = StatsObject->GetIntegerField(TEXT("coins"));
        int32 Armor = StatsObject->GetIntegerField(TEXT("armor"));
        int32 MagicResist = StatsObject->GetIntegerField(TEXT("magicResist"));

        UE_LOG(LogTemp, Warning, TEXT("Inventory loaded! %d"), Coins);

        APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
        if (!IsValid(PC))
            return;

        UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
        if (!ASC)
            return;
        AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

        if (IsValid(PC) && IsValid(PC->CharacterClassData))
        {

            for (const FCharacterClassData &ClassData : PC->CharacterClassData->CharacterClasses)
            {
                if (ClassData.ClassNameTag.ToString() == ClassName)
                {
                    PC->PlayerClassTag = ClassData.ClassNameTag;
                    PC->ForceNetUpdate();
                    PC->Server_ApplyCharacterClass(ClassData.ClassNameTag);
                    PCController->Server_SetupAttributes(Level, XP, Mana, Health, Coins, Armor, MagicResist);
                    break;
                }
            }
        }

        if (IsValid(PCController))
        {
            PCController->Server_SetupAttributes(Level, XP, Mana, Health, Coins, Armor, MagicResist);
        }

        if (IsValid(ASC))
        {
            ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));
        }

        const TSharedPtr<FJsonObject> *LocationObj;
        if (StatsObject->TryGetObjectField(TEXT("lastLocation"), LocationObj))
        {
            float X = (*LocationObj)->GetNumberField(TEXT("x"));
            float Y = (*LocationObj)->GetNumberField(TEXT("y"));
            float Z = (*LocationObj)->GetNumberField(TEXT("z"));

            FVector LastLocation(X, Y, Z);

            if (!LastLocation.IsZero() && IsValid(PCController))
            {
                FVector SafeLocation = PCController->RespawnComponent->GetCorrectZLocation(LastLocation);
                PCController->InteractionTraceComponent->Server_SetPlayerLocation(SafeLocation);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Stats not found - this is a new player!"));
    }

    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);
    if (IsValid(PCController))
    {
        PCController->bStatsLoaded = true;
        PCController->CheckAllLoaded();
        LoadInventory();
    }
}

// Inventory
void UM_BackendComponent::Server_GatherInventoryForSave_Implementation()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetCharacter());
    if (IsValid(PC) && IsValid(PC->InventoryComponent))
    {
        Client_SendInventoryToBackend(PC->InventoryComponent->Items, PC->InventoryComponent->WeaponSlot,
                                      PC->InventoryComponent->ArmorSlot);
    }
}

void UM_BackendComponent::Client_SendInventoryToBackend_Implementation(const TArray<FItemData> &ServerItems,
                                                                       FItemData WeaponSlot, FItemData ArmorSlot)
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        return;

    bool bHasItems = ServerItems.ContainsByPredicate([](const FItemData &Item) { return Item.IsValid(); });
    if (!bHasItems && !WeaponSlot.IsValid() && !ArmorSlot.IsValid())
        return;

    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    UM_BackendSubsystem *Backend = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!Backend)
        return;

    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> ItemsArray;

    for (const FItemData &Item : ServerItems)
    {
        if (!Item.IsValid())
            continue;
        TSharedPtr<FJsonObject> ItemObj = MakeShared<FJsonObject>();
        ItemObj->SetStringField(TEXT("itemId"), Item.ItemID.ToString());
        ItemObj->SetStringField(TEXT("itemName"), Item.ItemName.ToString());
        ItemObj->SetNumberField(TEXT("quantity"), Item.Quantity);
        ItemObj->SetBoolField(TEXT("equipped"), false);
        ItemObj->SetStringField(TEXT("slotType"), TEXT("Inventory"));
        ItemsArray.Add(MakeShared<FJsonValueObject>(ItemObj));
    }

    if (WeaponSlot.IsValid())
    {
        TSharedPtr<FJsonObject> WeaponObj = MakeShared<FJsonObject>();
        WeaponObj->SetStringField(TEXT("itemId"), WeaponSlot.ItemID.ToString());
        WeaponObj->SetStringField(TEXT("itemName"), WeaponSlot.ItemName.ToString());
        WeaponObj->SetNumberField(TEXT("quantity"), 1);
        WeaponObj->SetBoolField(TEXT("equipped"), true);
        WeaponObj->SetStringField(TEXT("slotType"), TEXT("Weapon"));
        ItemsArray.Add(MakeShared<FJsonValueObject>(WeaponObj));
    }

    if (ArmorSlot.IsValid())
    {
        TSharedPtr<FJsonObject> ArmorObj = MakeShared<FJsonObject>();
        ArmorObj->SetStringField(TEXT("itemId"), ArmorSlot.ItemID.ToString());
        ArmorObj->SetStringField(TEXT("itemName"), ArmorSlot.ItemName.ToString());
        ArmorObj->SetNumberField(TEXT("quantity"), 1);
        ArmorObj->SetBoolField(TEXT("equipped"), true);
        ArmorObj->SetStringField(TEXT("slotType"), TEXT("Armor"));
        ItemsArray.Add(MakeShared<FJsonValueObject>(ArmorObj));
    }

    Root->SetArrayField(TEXT("items"), ItemsArray);

    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/inventory"));
    Request->SetVerb(TEXT("PUT"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->SetContentAsString(Body);
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendComponent::OnInventorySaved);
    Request->ProcessRequest();
}

void UM_BackendComponent::RequestInventorySave()
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        return;
    Server_GatherInventoryForSave();
}

void UM_BackendComponent::StartInventorySaveTimer()
{
    if (!IsValid(GetOwner()))
        return;

    float RandomInventoryDelay = FMath::FRandRange(1.f, 30.f);

    GetOwner()->GetWorldTimerManager().SetTimer(InventorySaveTimer, this, &UM_BackendComponent::RequestInventorySave,
                                                40.f, true, RandomInventoryDelay);
}

void UM_BackendComponent::OnInventorySaved(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                                           TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save inventory!"));
        return;
    }
    if (Response->GetResponseCode() == 200)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory saved successfully!"));
    }
}

void UM_BackendComponent::LoadInventory()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
    {
        FTimerHandle RetryTimer;
        TWeakObjectPtr<UM_BackendComponent> WeakThis(this);
        GetWorld()->GetTimerManager().SetTimer(
            RetryTimer,
            [WeakThis]() {
                if (WeakThis.IsValid())
                    WeakThis->LoadInventory();
            },
            1.0f, false);
        return;
    }

    UM_BackendSubsystem *Backend = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!Backend || !Backend->bIsLoggedIn)
        return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/inventory"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *PlayerAuthToken));
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendComponent::OnInventoryLoaded);
    Request->ProcessRequest();
}

void UM_BackendComponent::OnInventoryLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                                            TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
                                            bool bWasSuccessful)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

    auto MarkLoadedAndReturn = [this, PCController]() {
        bInventoryLoaded = true;
        if (IsValid(PCController))
        {
            PCController->bInventoryLoaded = true;
            PCController->CheckAllLoaded();
        }
    };

    if (!bWasSuccessful || !Response.IsValid())
    {
        MarkLoadedAndReturn();
        return;
    }
    if (Response->GetResponseCode() != 200)
    {
        MarkLoadedAndReturn();
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        MarkLoadedAndReturn();
        return;
    }

    APlayerCharacter *PC = IsValid(OwnerController) ? Cast<APlayerCharacter>(OwnerController->GetPawn()) : nullptr;
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
    {
        MarkLoadedAndReturn();
        return;
    }

    const TSharedPtr<FJsonObject> *InventoryObj;
    if (!JsonObject->TryGetObjectField(TEXT("inventory"), InventoryObj))
    {
        MarkLoadedAndReturn();
        return;
    }

    const TArray<TSharedPtr<FJsonValue>> *ItemsArray;
    if (!(*InventoryObj)->TryGetArrayField(TEXT("items"), ItemsArray))
    {
        MarkLoadedAndReturn();
        return;
    }

    TArray<FItemData> LoadedItems;
    FItemData WeaponItem;
    FItemData ArmorItem;
    LoadedItems.SetNum(PC->InventoryComponent->Items.Num());

    int32 SlotIndex = 0;
    for (const TSharedPtr<FJsonValue> &ItemValue : *ItemsArray)
    {
        if (SlotIndex >= PC->InventoryComponent->Items.Num())
            break;
        const TSharedPtr<FJsonObject> *ItemObj;
        if (!ItemValue->TryGetObject(ItemObj))
        {
            SlotIndex++;
            continue;
        }

        FItemData NewItem;
        FString ItemId;
        (*ItemObj)->TryGetStringField(TEXT("itemId"), ItemId);
        NewItem.ItemID = FName(*ItemId);

        int32 Quantity;
        (*ItemObj)->TryGetNumberField(TEXT("quantity"), Quantity);
        NewItem.Quantity = Quantity;

        UAssetManager &AssetManager = UAssetManager::Get();
        TArray<FPrimaryAssetId> AssetIds;
        AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("M_Item_Details"), AssetIds);
        AssetManager.LoadPrimaryAssets(AssetIds, TArray<FName>());
        for (const FPrimaryAssetId &AssetId : AssetIds)
        {
            FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
            UM_Item_Details *ItemDetails = Cast<UM_Item_Details>(AssetPath.TryLoad());
            if (IsValid(ItemDetails) && ItemDetails->ItemID == NewItem.ItemID)
            {
                NewItem.ItemDataAsset = ItemDetails;
                NewItem.ItemName = ItemDetails->ItemName;
                NewItem.Icon = ItemDetails->Icon;
                NewItem.ItemType = ItemDetails->ItemType;
                NewItem.MaxStackSize = ItemDetails->MaxStackSize;
                NewItem.Socket = ItemDetails->Socket;
                NewItem.InteractiveClass = ItemDetails->ItemClassToSpawn;
                break;
            }
        }

        FString SlotType;
        (*ItemObj)->TryGetStringField(TEXT("slotType"), SlotType);

        if (SlotType == TEXT("Weapon"))
            WeaponItem = PC->InventoryComponent->WeaponSlot = NewItem;
        else if (SlotType == TEXT("Armor"))
            ArmorItem = PC->InventoryComponent->ArmorSlot = NewItem;
        else
        {
            LoadedItems[SlotIndex] = NewItem;
            SlotIndex++;
        }
    }

    for (int32 i = 0; i < PC->InventoryComponent->Items.Num(); i++)
        PC->InventoryComponent->Items[i] = FItemData();

    for (int32 i = 0; i < LoadedItems.Num(); i++)
        PC->InventoryComponent->Items[i] = LoadedItems[i];

    PC->InventoryComponent->OnInventoryChanged.Broadcast();

    if (IsValid(PCController))
    {
        PCController->InventoryActionsComponent->Server_UpdateInventoryAfterLoad(LoadedItems, WeaponItem, ArmorItem);
    }

    MarkLoadedAndReturn();
}

// Quests
void UM_BackendComponent::SavePlayerQuestsForBackEnd(FQuestData Quest)
{
    APlayerController *OwnerController = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    UM_BackendSubsystem *Backend = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!IsValid(Backend) || Backend->AuthToken.IsEmpty())
        return;

    TSharedPtr<FJsonObject> QuestObj = MakeShared<FJsonObject>();
    QuestObj->SetStringField(TEXT("questId"), Quest.QuestID);
    QuestObj->SetStringField(TEXT("questName"), Quest.QuestDescription);
    FString StatusStr = (Quest.Status == EQuestStatus::Completed) ? TEXT("Completed") : TEXT("Pending");
    QuestObj->SetStringField(TEXT("status"), StatusStr);

    FString Body;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(QuestObj.ToSharedRef(), Writer);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/quests"));
    Request->SetVerb(TEXT("PUT"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->SetContentAsString(Body);
    Request->ProcessRequest();
}

void UM_BackendComponent::LoadPlayerQuests()
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    UM_BackendSubsystem *Backend = OwnerController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!IsValid(Backend) || Backend->AuthToken.IsEmpty())
        return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/quests"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendComponent::OnPlayerQuestsLoaded);
    Request->ProcessRequest();
}

void UM_BackendComponent::OnPlayerQuestsLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                                               TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
                                               bool bWasSuccessful)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

    auto MarkLoadedAndReturn = [this, PCController]() {
        bQuestsLoaded = true;
        if (IsValid(PCController))
        {
            PCController->bQuestsLoaded = true;
            PCController->CheckAllLoaded();
        }
    };

    if (!bWasSuccessful || !Response.IsValid())
    {
        MarkLoadedAndReturn();
        return;
    }

    TArray<TSharedPtr<FJsonValue>> QuestsArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, QuestsArray))
    {
        MarkLoadedAndReturn();
        return;
    }

    APlayerCharacter *PlayerCharacter =
        IsValid(OwnerController) ? Cast<APlayerCharacter>(OwnerController->GetPawn()) : nullptr;
    if (!IsValid(PlayerCharacter))
    {
        MarkLoadedAndReturn();
        return;
    }

    UM_QuestComponent *QuestComp = PlayerCharacter->FindComponentByClass<UM_QuestComponent>();
    if (!IsValid(QuestComp))
    {
        MarkLoadedAndReturn();
        return;
    }

    for (TSharedPtr<FJsonValue> Value : QuestsArray)
    {
        TSharedPtr<FJsonObject> QuestObj = Value->AsObject();
        FString QuestID = QuestObj->GetStringField(TEXT("questId"));
        FString Status = QuestObj->GetStringField(TEXT("status"));

        FQuestData Quest;
        Quest.QuestID = QuestID;

        if (Status == TEXT("Completed"))
        {
            Quest.Status = EQuestStatus::Completed;
            QuestComp->CompletedQuestIDs.Add(QuestID);
            if (IsValid(PCController))
                PCController->QuestUIComponent->UpdateNPCQuestStatus(Quest, EQuestStatus::Completed);
        }
        else if (Status == TEXT("Pending"))
        {
            Quest = QuestComp->FindQuestInMasterAsset(QuestID);
            Quest.Status = EQuestStatus::Pending;
            QuestComp->ActiveQuestAssets.Add(Quest);
            if (IsValid(PCController))
            {
                PCController->QuestUIComponent->UpdateNPCQuestStatus(Quest, EQuestStatus::Pending);
                PCController->QuestUIComponent->Client_PendingQuestLoaded(Quest);
            }
            QuestComp->Server_AddActiveQuest(Quest);
        }
    }

    MarkLoadedAndReturn();
}