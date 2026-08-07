#include "M_PlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Characters/PlayerCharacter.h"
#include "Components/Widget.h"
#include "DataAsset/CharacterClasses.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Simple_Inventory/Widgets/M_Inventory.h"
#include "Widgets/M_AbilitiesContainer.h"
#include "Widgets/PlayerWidget/StatsWidget.h"

#include "Components/CapsuleComponent.h"
#include "Respawn/M_SpawnPoints.h"

#include "Attributes/M_AttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Simple_Inventory/Data/M_Item_Details.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Engine/AssetManager.h"

#include "AM_QuestNPC.h"
#include "M_CompletedQuestWidget.h"
#include "M_QuestComponent.h"
#include "M_QuestDialolgWidget.h"
#include "M_QuestsInfos.h"
#include "Weapons/WeaponBase.h"

#include "Backend/M_BackendSubsystem.h"
#include "Components/WidgetComponent.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "Characters/Enemies/M_Enemy_Area_Spawner.h"
#include "Characters/Enemy.h"
#include "Widgets/PlayerWidget/M_ShowNotification.h"

#include "Components/BoxComponent.h"
#include "Shop/M_MerchantNPC.h"
#include "Shop/UI/M_ShopWidget.h"

#include "Chat/M_ChatSubSystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Simple_Inventory/Widgets/M_QuantityWidget.h"

// Initialization
AM_PlayerController::AM_PlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AM_PlayerController::ShowLoadingScreen()
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC))
        return;

    if (PC->LoadingScreenClass && !PC->LoadingScreenWidget)
    {
        PC->LoadingScreenWidget = CreateWidget<UUserWidget>(this, PC->LoadingScreenClass);
        if (PC->LoadingScreenWidget)
            PC->LoadingScreenWidget->AddToViewport(999);
    }
    SetInputMode(FInputModeUIOnly());
    PC->GetCharacterMovement()->DisableMovement();
    bShowMouseCursor = false;
}

void AM_PlayerController::HideLoadingScreen()
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC))
        return;

    if (IsValid(PC->LoadingScreenWidget))
    {
        PC->LoadingScreenWidget->RemoveFromParent();
        PC->LoadingScreenWidget = nullptr;
    }
    SetInputMode(FInputModeGameOnly());
    PC->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
    // bShowMouseCursor = true;
}

void AM_PlayerController::CheckAllLoaded()
{

    FString MapName = GetWorld()->GetMapName();
    if (bStatsLoaded && bInventoryLoaded && bQuestsLoaded && !MapName.Contains("Character_Selection") &&
        !MapName.Contains("LoginLayer") && bOnASCReadyForAbilities)
        HideLoadingScreen();
}

void AM_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());

    PlayTheSound();
    FString MapName = GetWorld()->GetMapName();

    if (MapName.Contains("Character_Selection"))
    {
        if (PC)
        {
            PC->GetCharacterMovement()->DisableMovement();
            PC->GetCharacterMovement()->StopMovementImmediately();
        }

        FInputModeUIOnly InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }

    // LoadPlayerData();

    if (IsValid(PC))
    {
        PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
        PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);

        UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
    }

    if (IsLocalController() && !MapName.Contains("Character_Selection") && !MapName.Contains("LoginLayer"))
        ShowLoadingScreen();

    bStatsLoaded = false;
    bInventoryLoaded = false;
    bQuestsLoaded = false;

    LoadPlayerData();
    if (IsLocalController())
    {

        if (UGameInstance *GI = GetGameInstance())
            if (UM_ChatSubSystem *ChatSub = GI->GetSubsystem<UM_ChatSubSystem>())
            {
                ChatSub->ConnectToChat();
            }
        LoadPlayerQuests();

        float RandomInvDelay = FMath::FRandRange(1.f, 30.f);
        GetWorld()->GetTimerManager().SetTimer(
            InventorySaveTimer, this, &AM_PlayerController::Client_RequestInventorySave, 25.f, true, RandomInvDelay);

        float RandomStatsDelay = FMath::FRandRange(1.f, 30.f);
        GetWorld()->GetTimerManager().SetTimer(StatsSaveTimer, this, &AM_PlayerController::Client_RequestStatsSave,
                                               40.f, true, RandomStatsDelay);
    }
}
void AM_PlayerController::Tick(float DeltaSeconds)
{
    TraceForItem();
}
void AM_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        for (UInputMappingContext *Context : AddedMappingContexts)
        {
            if (IsValid(Context))
            {
                Subsystem->AddMappingContext(Context, 0);
            }
        }
    }
    UEnhancedInputComponent *EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!IsValid(EIC))
        return;

    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Move);
    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Look);
    EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AM_PlayerController::Jump);
    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AM_PlayerController::StopJumping);
    EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AM_PlayerController::ToggleInventory);
    EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AM_PlayerController::TryInteract);
}
void AM_PlayerController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    APlayerCharacter *PC = Cast<APlayerCharacter>(InPawn);
    if (!IsValid(PC))
        return;
    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (IsValid(Backend) && !Backend->AuthToken.IsEmpty())
    {
        PlayerAuthToken = Backend->AuthToken;
    }

    PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
    PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);
    PC->InventoryComponent->OnInventoryFull.AddDynamic(this, &AM_PlayerController::OnInventoryFull);

    UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        OnASCReady(ASC, PC->GetAttributeSet());
    }

    // FString MapName = GetWorld()->GetMapName();
    // if (!MapName.Contains("Character_Selection"))

    if (IsLocalController())
    {
        if (IsValid(InventoryWidgetClass))
        {
            InventoryWidget = CreateWidget<UM_Inventory>(this, InventoryWidgetClass);
            if (IsValid(InventoryWidget))
            {
                InventoryWidget->InitializeInventory(PC->InventoryComponent);
                InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
                InventoryWidget->AddToViewport(10);
            }
        }
    }

    //	LoadPlayerQuests();

    if (IsLocalController())
    {
        if (!IsValid(QuantityWidgetClass))
            return;
        if (!IsValid(QuantityWidget))
        {
            QuantityWidget = CreateWidget<UM_QuantityWidget>(this, QuantityWidgetClass);
            QuantityWidget->AddToViewport(15);
        }
        if (!IsValid(QuantityWidget))
            return;
        QuantityWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}
void AM_PlayerController::OnASCReady(UAbilitySystemComponent *ASC, UAttributeSet *AS)
{
    // if (!IsLocalController()) return;
    if (!IsValid(ASC))
        return;
    FString MapName = GetWorld()->GetMapName();
    if (MapName.Contains("Untitledeeee") || MapName.Contains("MythionMap"))
    {
        if (IsValid(StatsWidgetClass) && !IsValid(StatsWidget))
        {
            StatsWidget = CreateWidget<UStatsWidget>(this, StatsWidgetClass);
            if (IsValid(StatsWidget))
            {
                StatsWidget->AddToViewport();
                StatsWidget->InitializeWidget(ASC);
            }
        }
    }
}
void AM_PlayerController::HandleChatConnectedTest()
{
    if (UGameInstance *GI = GetGameInstance())
    {
        if (UM_ChatSubSystem *ChatSub = GI->GetSubsystem<UM_ChatSubSystem>())
        {
            ChatSub->SendChatMessage(TEXT("testFromUE"), TEXT("hello from UE"));
        }
    }
}

// Actions
void AM_PlayerController::Move(const FInputActionValue &Value)
{

    APlayerCharacter *Char = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(Char))
        return;

    UAbilitySystemComponent *ASC = Char->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Attacking")))
    {
        return;
    }
    if (!IsValid(GetPawn()))
        return;

    const FVector2D MovementVector = Value.Get<FVector2D>();
    const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
    GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}
void AM_PlayerController::Look(const FInputActionValue &Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddYawInput(LookVector.X);
    AddPitchInput(LookVector.Y);
}
void AM_PlayerController::Jump()
{
    if (!IsValid(GetCharacter()))
        return;
    GetCharacter()->Jump();
}
void AM_PlayerController::StopJumping()
{
    if (!IsValid(GetCharacter()))
        return;
    GetCharacter()->StopJumping();
}
void AM_PlayerController::TryInteract()
{
    if (!FocusedItem.IsValid())
        return;
    AM_Interactive_Item *Item = Cast<AM_Interactive_Item>(FocusedItem.Get());
    if (FocusedItem.IsValid())
    {
        Server_Interact(Item);
    }

    AAM_QuestNPC *NPC = Cast<AAM_QuestNPC>(FocusedItem.Get());

    if (IsValid(NPC))
    {

        //	ShowQuestDialog(NPC,NPC->CurrentQuestData);

        NPC->Interact(GetPawn());
    }

    AM_MerchantNPC *Merchant = Cast<AM_MerchantNPC>(FocusedItem.Get());
    if (Merchant)
    {
        Merchant->Interact(GetPawn());
    }
}
void AM_PlayerController::ToggleInventory()
{

    if (!IsLocalController())
        return;
    if (!IsValid(InventoryWidget))
        return;

    bool bIsVisible = InventoryWidget->GetVisibility() == ESlateVisibility::Visible;
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC))
        return;

    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    if (!IsValid(PS))
        return;
    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();

    if (!IsValid(ASC))
        return;

    if (bIsVisible)
    {

        InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
        if (!IsValid(QuantityWidget))
            return;
        QuantityWidget->SetbIsOpen(false);
        QuantityWidget->SetVisibility(ESlateVisibility::Hidden);
        ClientPlaySoundAtLocation(CloseSound, PC->GetActorLocation(), 1, 1);

        // ASC->RemoveActiveGameplayEffect(InventoryEffectHandle);
    }
    else
    {

        InventoryWidget->SetVisibility(ESlateVisibility::Visible);
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        if (!IsValid(PC))
            return;

        ClientPlaySoundAtLocation(Sound, PC->GetActorLocation(), 1, 1);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}
void AM_PlayerController::OnInventoryFull()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Inventory is full!"));
}

// Inventory &  Backend
void AM_PlayerController::Server_RequestInventoryData_Implementation()
{

    APlayerCharacter *PC = Cast<APlayerCharacter>(GetCharacter());

    if (IsValid(PC) && IsValid(PC->InventoryComponent))
    {
        Client_ReceiveInventoryData(PC->InventoryComponent->Items, PC->InventoryComponent->WeaponSlot,
                                    PC->InventoryComponent->ArmorSlot);
    }
}
void AM_PlayerController::Client_ReceiveInventoryData_Implementation(const TArray<FItemData> &ServerItems,
                                                                     FItemData WeaponSlot, FItemData ArmorSlot)
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
    {
        return;
    }

    bool bHasItems = ServerItems.ContainsByPredicate([](const FItemData &Item) { return Item.IsValid(); });
    if (!bHasItems && !WeaponSlot.IsValid() && !ArmorSlot.IsValid())
        return;

    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
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

    // Armor slot
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
    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnInventorySaved);
    Request->ProcessRequest();
}
void AM_PlayerController::Client_RequestInventorySave()
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
    {
        return;
    }
    Server_RequestInventoryData();
}
void AM_PlayerController::OnInventorySaved(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
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
void AM_PlayerController::LoadInventory()
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
    {

        FTimerHandle RetryTimer;
        GetWorld()->GetTimerManager().SetTimer(RetryTimer, this, &AM_PlayerController::LoadInventory, 1.0f, false);
        return;
    }
    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!Backend || !Backend->bIsLoggedIn)
        return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/inventory"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *PlayerAuthToken));
    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnInventoryLoaded);
    Request->ProcessRequest();
}
void AM_PlayerController::OnInventoryLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                                            TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
                                            bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
        return;
    }
    if (Response->GetResponseCode() != 200)
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
        return;
    }

    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
        return;
    }

    const TSharedPtr<FJsonObject> *InventoryObj;
    if (!JsonObject->TryGetObjectField(TEXT("inventory"), InventoryObj))
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
        return;
    }

    const TArray<TSharedPtr<FJsonValue>> *ItemsArray;
    if (!(*InventoryObj)->TryGetArrayField(TEXT("items"), ItemsArray))
    {
        bInventoryLoaded = true;
        CheckAllLoaded();
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

    // Reset και apply μαζί
    for (int32 i = 0; i < PC->InventoryComponent->Items.Num(); i++)
        PC->InventoryComponent->Items[i] = FItemData();

    for (int32 i = 0; i < LoadedItems.Num(); i++)
        PC->InventoryComponent->Items[i] = LoadedItems[i];

    PC->InventoryComponent->OnInventoryChanged.Broadcast();
    Server_UpdateInventoryAfterLoad(LoadedItems, WeaponItem, ArmorItem);

    bInventoryLoaded = true;
    CheckAllLoaded();
}

// Stats & Backend
void AM_PlayerController::Server_SavePlayerStats_Implementation()
{
    APlayerCharacter *Char = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(Char))
        return;

    UAbilitySystemComponent *AbilitySC = Char->GetAbilitySystemComponent();
    if (!IsValid(AbilitySC))
        return;

    FVector Lastlocation = Char->GetActorLocation();

    if (IsValid(Char) && IsValid(Char->GetAbilitySystemComponent()))
    {
        float CurrentHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
        int32 CurrentLevel = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
        float CurrentMana = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetManaAttribute());
        int32 CurrentCoins = FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute()));
        int32 CurrentArmor = FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute()));
        int32 CurrentMagicResist =
            FMath::RoundToInt(AbilitySC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute()));
        float CurrentXP = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());

        if (CurrentHealth <= 0.f)
        {
            return;
        }

        Client_ReceivePlayerStats(CurrentLevel, CurrentXP, CurrentCoins, CurrentHealth, CurrentMana, CurrentArmor,
                                  CurrentMagicResist, Lastlocation);
    }
}
void AM_PlayerController::Client_ReceivePlayerStats_Implementation(int32 Level, float XP, int32 Gold, float Health,
                                                                   float Mana, int32 Armor, int32 MagicResistance,
                                                                   FVector LastLocation)
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        return;

    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!Backend || Backend->AuthToken.IsEmpty())
        return;

    TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();

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
    UE_LOG(LogTemp, Warning, TEXT("Saving stats - Body: %s"), *Body);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/stats"));
    Request->SetVerb(TEXT("PUT"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->SetContentAsString(Body);

    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnPlayerStatsSaved);
    Request->ProcessRequest();
}
void AM_PlayerController::OnPlayerStatsSaved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
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
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save stats - server responded with code %d"),
               Response->GetResponseCode());
    }
}
void AM_PlayerController::Client_RequestStatsSave()
{
    if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
    {
        return;
    }
    Server_SavePlayerStats();
}
void AM_PlayerController::OnPlayerDataLoaded(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
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

        UE_LOG(LogTemp, Warning, TEXT("Inventory loaded! %d"), Coins);

        // TODO: Apply these to your AttributeSet
        APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
        if (!IsValid(PC))
            return;

        UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
        if (!ASC)
            return;
        const UM_AttributeSet *ConstAttributeSet =
            Cast<UM_AttributeSet>(ASC->GetAttributeSet(UM_AttributeSet::StaticClass()));
        if (!ConstAttributeSet)
            return;

        UM_AttributeSet *AttributeSet = const_cast<UM_AttributeSet *>(ConstAttributeSet);

        /*
        if(IsValid(AttributeSet))
        {
            AttributeSet->SetLevel(Level);
            AttributeSet->SetXp(XP);
            AttributeSet->SetHealth(Health);
            AttributeSet->SetMana(Mana);

        }
        */

        if (IsValid(PC) && IsValid(PC->CharacterClassData))
        {
            for (const FCharacterClassData &ClassData : PC->CharacterClassData->CharacterClasses)
            {
                if (ClassData.ClassNameTag.ToString() == ClassName)
                {
                    PC->PlayerClassTag = ClassData.ClassNameTag;
                    PC->ForceNetUpdate();
                    PC->Server_ApplyCharacterClass(ClassData.ClassNameTag);
                    Server_SetupAttributes(Level, XP, Mana, Health, Coins);
                    break;
                }
            }
        }

        Server_SetupAttributes(Level, XP, Mana, Health, Coins);

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

            if (!LastLocation.IsZero())
            {
                FVector SafeLocation = GetCorrectZLocation(LastLocation);
                Server_SetPlayerLocation(SafeLocation);
            }
        }

        // Server_SetupAttributes(Level, XP, Mana, Health);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Stats not found - this is a new player!"));
    }

    bStatsLoaded = true;
    CheckAllLoaded();
    LoadInventory();
}
void AM_PlayerController::Client_SetTokenAndTravel_Implementation(const FString &Token, const FString &ServerIP)
{
    PlayerAuthToken = Token;

    // UM_BackendSubsystem* Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    //	if (IsValid(Backend))
    //	Backend->AuthToken = Token;

    // ClientTravel(ServerIP, ETravelType::TRAVEL_Absolute);
}
void AM_PlayerController::LoadPlayerData()
{

    // if (GetNetMode() == NM_ListenServer && IsLocalController()) return;
    UM_BackendSubsystem *BackendSubsystem = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();

    if (!BackendSubsystem || !BackendSubsystem->bIsLoggedIn)
    {
        UE_LOG(LogTemp, Error, TEXT("Not logged in! Cannot load stats."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("LoadPlayerData - Token: %s"), *PlayerAuthToken);
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

    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnPlayerDataLoaded);
    Request->ProcessRequest();
}

// Respawn
void AM_PlayerController::LoadPlayerForRespawn()
{
    UM_BackendSubsystem *BackendSubsystem = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();

    if (!BackendSubsystem || !BackendSubsystem->bIsLoggedIn)
    {
        UE_LOG(LogTemp, Error, TEXT("Not logged in! Cannot load stats."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("LoadPlayerData - Token: %s"), *PlayerAuthToken);
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

    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnPlayerDataForRespawn);
    Request->ProcessRequest();
}
void AM_PlayerController::OnPlayerDataForRespawn(TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> Request,
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

    if (Response->GetResponseCode() == 200)
    {

        FString ClassName;
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDataForRespawn - Response: %s"), *Response->GetContentAsString());
        TSharedPtr<FJsonObject> StatsObject = JsonObject->GetObjectField(TEXT("stats"));

        int32 Level = StatsObject->GetIntegerField(TEXT("level"));
        int32 XP = StatsObject->GetIntegerField(TEXT("xp"));
        int32 Coins = StatsObject->GetIntegerField(TEXT("coins"));
        ClassName = StatsObject->GetStringField(TEXT("className"));
        UE_LOG(LogTemp, Warning, TEXT("OnPlayerDataForRespawn - Level: %d"), Level);

        const TSharedPtr<FJsonObject> *LocationObj;
        if (StatsObject->TryGetObjectField(TEXT("lastLocation"), LocationObj))
        {
            float X = (*LocationObj)->GetNumberField(TEXT("x"));
            float Y = (*LocationObj)->GetNumberField(TEXT("y"));
            float Z = (*LocationObj)->GetNumberField(TEXT("z"));

            FVector LastLocation(X, Y, Z);

            // Μόνο αν δεν είναι 0,0,0 (new player)
            if (!LastLocation.IsZero())
                Server_SetPlayerLocation(LastLocation);
        }

        APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
        if (!IsValid(PC))
            return;
        UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
        if (!ASC)
            return;
        /*
                if (IsValid(ASC))
                {
                    ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));
                }
        */
        const UM_AttributeSet *ConstAttributeSet =
            Cast<UM_AttributeSet>(ASC->GetAttributeSet(UM_AttributeSet::StaticClass()));
        if (!ConstAttributeSet)
            return;

        UM_AttributeSet *AttributeSet = const_cast<UM_AttributeSet *>(ConstAttributeSet);

        bRespawned = true;
        if (IsValid(PC) && IsValid(PC->CharacterClassData))
        {
            UE_LOG(LogTemp, Warning, TEXT("Respawning cLASSED: %s"), *ClassName);
            for (const FCharacterClassData &ClassData : PC->CharacterClassData->CharacterClasses)
            {

                if (ClassData.ClassNameTag.ToString() == ClassName)
                {
                    PC->PlayerClassTag = ClassData.ClassNameTag;
                    PC->ForceNetUpdate();
                    UE_LOG(LogTemp, Warning, TEXT("Respawning cLASSED: %s"), *ClassName);
                    Server_SetupAttributes(Level, XP, 1.f, 1.f, Coins);
                    break;
                }
            }
        }
    }
}

// Quests & Backend

// Quests and Widgets
void AM_PlayerController::Client_ShowQuestDialog_Implementation(AAM_QuestNPC *NPC, FQuestData QuestData)
{
    if (!IsValid(QuestWidgetDialogClass))
        return;

    if (!IsValid(QuestWidgetDialog))
    {
        QuestWidgetDialog = CreateWidget<UM_QuestDialolgWidget>(this, QuestWidgetDialogClass);
        QuestWidgetDialog->AddToViewport();
        QuestWidgetDialog->SetVisibility(ESlateVisibility::Hidden);
    }

    QuestWidgetDialog->InitDialog(NPC, QuestData);

    if (QuestWidgetDialog->GetIsDialogOpen())
        QuestWidgetDialog->CloseDialog();
    else
        QuestWidgetDialog->OpenDialog();
}
void AM_PlayerController::Client_PendingQuestLoaded_Implementation(FQuestData QuestAsset)
{
    if (!IsLocalController())
        return;

    if (!IsValid(StatsWidget) || !IsValid(StatsWidget->QuestInfoWidget))
        return;

    UM_QuestsInfos *QuestInfoWidgeOpen = StatsWidget->QuestInfoWidget;

    if (!IsValid(QuestInfoWidgeOpen))
        return;
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (IsValid(PC))
    {
        UM_QuestComponent *QuestComp = PC->FindComponentByClass<UM_QuestComponent>();
        if (IsValid(QuestComp))
        {
            QuestComp->OnQuestUpdated.RemoveDynamic(QuestInfoWidgeOpen, &UM_QuestsInfos::UpdateQuest);
            QuestComp->OnQuestUpdated.AddDynamic(QuestInfoWidgeOpen, &UM_QuestsInfos::UpdateQuest);
        }
    }

    QuestInfoWidgeOpen->UpdateQuest(QuestAsset, 0);
    QuestInfoWidgeOpen->OpenQuestInfo();
    UpdateNPCQuestStatus(QuestAsset, EQuestStatus::Pending);
    SavePlayerQuestsForBackEnd(QuestAsset);
}
void AM_PlayerController::UpdateNPCQuestStatus(FQuestData Quest, EQuestStatus Status)
{
    TArray<AActor *> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAM_QuestNPC::StaticClass(), FoundNPCs);

    if (FoundNPCs.Num() == 0)
    {
        FQuestData QuestCopy = Quest;
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(
            RetryHandle, [this, QuestCopy, Status]() { UpdateNPCQuestStatus(QuestCopy, Status); }, 0.5f, false);
        return;
    }

    for (AActor *Actor : FoundNPCs)
    {
        AAM_QuestNPC *NPC = Cast<AAM_QuestNPC>(Actor);
        if (IsValid(NPC) && NPC->NPCQuestID.Equals(Quest.QuestID))
        {
            NPC->UpdateQuestStatusIcon(Status);
            break;
        }
    }
}
void AM_PlayerController::ShowCompletedQuest(FQuestData Quest)
{
    if (!IsValid(QuestCompletedWidget))
    {
        QuestCompletedWidget = CreateWidget<UM_CompletedQuestWidget>(this, QuestCompletedClass);
        QuestCompletedWidget->AddToViewport(10);
    }
    QuestCompletedWidget->SetText(Quest);
    QuestCompletedWidget->SetVisibility(ESlateVisibility::Visible);

    FTimerHandle HideTimer;
    GetWorld()->GetTimerManager().SetTimer(
        HideTimer,
        [this]() {
            if (IsValid(QuestCompletedWidget))
                QuestCompletedWidget->SetVisibility(ESlateVisibility::Hidden);
        },
        4.f, false);
}
void AM_PlayerController::Server_CompleteQuest_Implementation(FQuestData QuestAsset)
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC))
        return;
    UM_QuestComponent *QuestComp = PC->FindComponentByClass<UM_QuestComponent>();
    if (!IsValid(QuestComp))
        return;

    // QuestComp->CompleteQuest(QuestAsset);

    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    if (!IsValid(PS))
        return;
    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), QuestAsset.CoinReward);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}
void AM_PlayerController::Client_CompleteQuest_Implementation(FQuestData QuestAsset)
{

    if (IsValid(StatsWidget) && IsValid(StatsWidget->QuestInfoWidget))
        StatsWidget->QuestInfoWidget->CloseQuestInfo();

    UpdateNPCQuestStatus(QuestAsset, EQuestStatus::Completed);
    SavePlayerQuestsForBackEnd(QuestAsset);
    ShowCompletedQuest(QuestAsset);
}
void AM_PlayerController::Client_UpdateKills_Implementation(FQuestData Quest, int32 CurrentKills)
{
    if (!IsValid(StatsWidget) || !IsValid(StatsWidget->QuestInfoWidget))
        return;
    StatsWidget->QuestInfoWidget->UpdateQuest(Quest, CurrentKills);
}
// Quests and Backend
void AM_PlayerController::SavePlayerQuestsForBackEnd(FQuestData Quest)
{
    /*
        if (GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_ListenServer)
        {
            return;
        }
    */

    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
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
void AM_PlayerController::LoadPlayerQuests()
{
    UM_BackendSubsystem *Backend = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    if (!IsValid(Backend) || Backend->AuthToken.IsEmpty())
        return;

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Backend->ServerURL + TEXT("/api/player/quests"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
    Request->OnProcessRequestComplete().BindUObject(this, &AM_PlayerController::OnPlayerQuestsLoaded);
    Request->ProcessRequest();
}
void AM_PlayerController::OnPlayerQuestsLoaded(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request,
                                               TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response,
                                               bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
        return;

    TArray<TSharedPtr<FJsonValue>> QuestsArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, QuestsArray))
        return;

    APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PlayerCharacter))
        return;
    UM_QuestComponent *QuestComp = PlayerCharacter->FindComponentByClass<UM_QuestComponent>();
    if (!IsValid(QuestComp))
        return;
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
            UpdateNPCQuestStatus(Quest, EQuestStatus::Completed);
        }
        else if (Status == TEXT("Pending"))
        {
            Quest = QuestComp->FindQuestInMasterAsset(QuestID);
            Quest.Status = EQuestStatus::Pending;
            QuestComp->ActiveQuestAssets.Add(Quest);
            UpdateNPCQuestStatus(Quest, EQuestStatus::Pending);
            Client_PendingQuestLoaded(Quest);
            QuestComp->Server_AddActiveQuest(Quest);
        }
    }

    bQuestsLoaded = true;
    CheckAllLoaded();
}
// Inventory & Interaction
void AM_PlayerController::TryInitInventory()
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC) || !IsValid(PC->InventoryComponent))
        return;

    GetWorld()->GetTimerManager().ClearTimer(InventoryInitTimer);

    if (IsValid(InventoryWidgetClass))
    {
        InventoryWidget = CreateWidget<UM_Inventory>(this, InventoryWidgetClass);
        if (IsValid(InventoryWidget))
        {
            InventoryWidget->InitializeInventory(PC->InventoryComponent);
            InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
            InventoryWidget->AddToViewport();
        }
    }
}
bool AM_PlayerController::Server_UpdateInventoryAfterLoad_Validate(const TArray<FItemData> &LoadedItems,
                                                                   FItemData WeaponItem, FItemData ArmorItem)
{
    return true;
}
void AM_PlayerController::Server_UpdateInventoryAfterLoad_Implementation(const TArray<FItemData> &LoadedItems,
                                                                         FItemData WeaponItem, FItemData ArmorItem)
{

    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_Interact_Implementation(AM_Interactive_Item *Item)
{
    if (!IsValid(Item))
        return;
    Item->Interact(GetPawn());
}

// For Items
void AM_PlayerController::Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity)
{

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_SellItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent))
        return;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FItemData SoldItem = PlayerChar->InventoryComponent->Items[SlotIndex];
    if (!SoldItem.IsValid() || !IsValid(SoldItem.ItemDataAsset))
        return;

    int32 SellQuantity = FMath::Clamp(Quantity, 1, SoldItem.Quantity);

    // Remove item
    if (SellQuantity >= SoldItem.Quantity)
        PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
    else
    {
        PlayerChar->InventoryComponent->Items[SlotIndex].Quantity -= SellQuantity;
        PlayerChar->InventoryComponent->OnInventoryChanged.Broadcast();
    }

    // Add coins
    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), SoldItem.ItemDataAsset->SellPrice * SellQuantity);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}
void AM_PlayerController::Server_SellWeapon_Implementation()
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_SellArmor_Implementation()
{

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_UseItem_Implementation(int32 SlotIndex)
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
        Client_CantUseItem();
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

bool AM_PlayerController::IsAttributeFull(EItemStats ItemStats, UAbilitySystemComponent *ASC)
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

// For Weapon Slot
void AM_PlayerController::Server_DropWeaponSlot_Implementation()
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_DropArmorSlot_Implementation()
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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

void AM_PlayerController::Server_UseWeaponSlot_Implementation()
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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

        /*
        float CurrentHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
        float MaxHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
        if (CurrentHealth >= MaxHealth)
        {
            Client_CantUseItem();
            return;
        }
        */

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
void AM_PlayerController::Server_UseArmorsSlot_Implementation()
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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

        /*
        float CurrentHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
        float MaxHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
        if (CurrentHealth >= MaxHealth)
        {
            Client_CantUseItem();
            return;
        }
        */

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

void AM_PlayerController::Client_ShowNotification_Implementation(const FText &Message)
{
    if (!IsValid(NotificationWidget))
    {
        NotificationWidget = CreateWidget<UM_ShowNotification>(this, NotificationWidgetClass);
        NotificationWidget->AddToViewport();
    }
    NotificationWidget->ShowMessage(Message);
}
void AM_PlayerController::Client_ShowItemName_Implementation(const FText &ItemName)
{
    if (!IsValid(NotificationWidget))
    {
        NotificationWidget = CreateWidget<UM_ShowNotification>(this, NotificationWidgetClass);
        NotificationWidget->AddToViewport();
    }
    NotificationWidget->ShowNameOfItem(ItemName);
}
void AM_PlayerController::Client_CantUseItem_Implementation()
{
    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("You can't use this item!"));
}
void AM_PlayerController::Server_ApplyInventoryEffect_Implementation(bool bOpen)
{
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();

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
void AM_PlayerController::TraceForItem()
{
    if (!IsLocalController())
        return;

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
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
void AM_PlayerController::Server_SetPlayerLocation_Implementation(FVector Location)
{
    APlayerCharacter *Char = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(Char))
        return;

    TArray<AActor *> Spawners;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AM_Enemy_Area_Spawner::StaticClass(), Spawners);

    FVector FinalLocation = Location;

    UE_LOG(LogTemp, Warning, TEXT("Server_SetPlayerLocation called with Location: %s"), *Location.ToString());
    for (AActor *Spawner : Spawners)
    {
        AM_Enemy_Area_Spawner *SpawnAreaD = Cast<AM_Enemy_Area_Spawner>(Spawner);
        if (!IsValid(SpawnAreaD))
            continue;

        FVector BoxOrigin = SpawnAreaD->SpawnArea->GetComponentLocation();
        FVector BoxExtent = SpawnAreaD->SpawnArea->GetScaledBoxExtent();

        UE_LOG(LogTemp, Warning, TEXT("Box Origin: %s, Extent: %s, PlayerLoc: %s"), *BoxOrigin.ToString(),
               *BoxExtent.ToString(), *Location.ToString());
        if (IsValid(SpawnAreaD) && SpawnAreaD->IsLocationInsideBox(Location))
        {
            FinalLocation = SpawnAreaD->GetSafeSpawnLocation(Location);
            UE_LOG(LogTemp, Warning, TEXT("Inside the box  called with Location: %s"), *FinalLocation.ToString());
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Server_SetPlayerLocation called with Location: %s"), *Location.ToString());
    Char->SetActorLocation(FinalLocation);
}

// Respawn
void AM_PlayerController::Server_Respawn_Implementation()
{
    APlayerCharacter *Char = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(Char))
        return;

    FVector TargetLocation = GetSafeRespawnPoint(Char->DeathLocation);

    Char->SetActorLocationAndRotation(TargetLocation, Char->GetActorRotation(), false, nullptr,
                                      ETeleportType::TeleportPhysics);

    EnemiesSpottingMe.Empty();
    PlayTheSound();

    Char->GetMesh()->SetSimulatePhysics(false);
    Char->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Char->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Char->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

    UAbilitySystemComponent *ASC = Char->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Status.Dead"))));
        ASC->RemoveActiveEffects(Query);

        if (IsValid(Char->InitStatsEffect))
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Char->InitStatsEffect, 1, Context);
            float MaxHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
            float MaxMana = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxManaAttribute());
            float CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
            float CurrentXP = ASC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());
            float CurrentXPMax = ASC->GetNumericAttribute(UM_AttributeSet::GetXpMaxAttribute());
            float CurrentCoins = ASC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute());
            float CurrentArmor = ASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());
            float MagicResistance = ASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());
            float CurrentEnergy = 500.f;
            float CurrentMaxEnergy = 500.f;

            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.MaxMana"), MaxMana);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.Level"), CurrentLevel);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.XP"), CurrentXP);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.XpMax"), CurrentXPMax);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins.Initialize"), CurrentCoins);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.Armor"), CurrentArmor);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.MagicResist"), MagicResistance);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.Energy"), CurrentEnergy);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.MaxEnergy"), CurrentMaxEnergy);

            // Health/Mana = Max (full heal)
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.Health"), MaxHealth);
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
                Spec, FGameplayTag::RequestGameplayTag("Data.Mana"), MaxMana);

            ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
        bool bStillHasDeadTag = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));
        UE_LOG(LogTemp, Warning, TEXT("After respawn, still has Status.Dead tag: %s"),
               bStillHasDeadTag ? TEXT("YES - PROBLEM") : TEXT("No, clean"));
    }
    TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(FGameplayEffectQuery());
    UE_LOG(LogTemp, Warning, TEXT("Active Effects Count: %d"), ActiveEffects.Num());
    for (auto &Handle : ActiveEffects)
    {
        const FActiveGameplayEffect *ActiveGE = ASC->GetActiveGameplayEffect(Handle);
        if (ActiveGE)
        {
            UE_LOG(LogTemp, Warning, TEXT("Effect: %s, IsInhibited: %s"), *ActiveGE->Spec.Def->GetName(),
                   ActiveGE->bIsInhibited ? TEXT("YES") : TEXT("NO"));
        }
    }
    Client_RequestStatsSave();
    Client_HideRespawnWidget();
}

void AM_PlayerController::Client_ShowRespawnWidget_Implementation()
{

    if (!RespawnWidgetClass)
        return;

    FTimerHandle DelayTimer;
    GetWorld()->GetTimerManager().SetTimer(
        DelayTimer,
        [this]() {
            if (!IsValid(RespawnWidget))
            {
                RespawnWidget = CreateWidget<UM_RespawnWidget>(this, RespawnWidgetClass);
                if (!IsValid(RespawnWidget))
                    return;
                RespawnWidget->AddToViewport();
            }
            RespawnWidget->SetVisibility(ESlateVisibility::Visible);
            SetInputMode(FInputModeUIOnly());
            bShowMouseCursor = true;
        },
        3.f, false);
}
void AM_PlayerController::Client_HideRespawnWidget_Implementation()
{
    if (IsValid(RespawnWidget))
        RespawnWidget->SetVisibility(ESlateVisibility::Hidden);
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

// Shop
void AM_PlayerController::Client_OpenShop_Implementation(AM_MerchantNPC *Merchant)
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());

    if (IsLocalController())
    {
        if (!IsValid(ShopWidget))
        {
            if (IsValid(ShopWidgetClass))
            {
                ShopWidget = CreateWidget<UM_ShopWidget>(this, ShopWidgetClass);
                if (IsValid(ShopWidget))
                {
                    ShopWidget->AddToViewport(2);
                    ShopWidget->SetVisibility(ESlateVisibility::Hidden);
                }
            }
            else
            {
                return;
            }
        }
    }

    if (IsValid(ShopWidget))
    {

        if (!ShopWidget->bOpenShopUI)
        {
            ShopWidget->ShowShopUI();
            ShopWidget->InitShop(Merchant, PlayerChar->PlayerClassTag);

            if (IsValid(QuantityWidget))
            {

                QuantityWidget->bIsSellMode = true;
            }

            if (IsValid(GreetingSoundShop))
            {
                UGameplayStatics::PlaySoundAtLocation(this, GreetingSoundShop, PlayerChar->GetActorLocation());
            }

            if (IsValid(InventoryWidget))
            {
                InventoryWidget->SetVisibility(ESlateVisibility::Visible);
                bShowMouseCursor = true;
                FInputModeGameAndUI InputMode;
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                SetInputMode(InputMode);
            }
        }
        else
        {
            ShopWidget->HideShopUI();
            if (IsValid(QuantityWidget))
                QuantityWidget->bIsSellMode = false;
            if (IsValid(InventoryWidget))
                InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
            SetInputMode(FInputModeGameOnly());
            bShowMouseCursor = false;
            if (IsValid(GoodByeSoundShop))
            {
                UGameplayStatics::PlaySoundAtLocation(this, GoodByeSoundShop, PlayerChar->GetActorLocation());
            }
        }
    }
}
void AM_PlayerController::Server_BuyItemFromShop_Implementation(FItemData Item)
{
    if (!Item.IsValid())
        return;
    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PC))
        return;

    UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    int32 Coins = ASC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute());

    if (Coins < Item.ItemDataAsset->BuyPrice)
    {
        Client_ShowNotification(FText::FromString("Not enough coins!"));
        return;
    }
    else
    {

        if (Item.ItemType == EItemType::Weapon)
        {
            if (PC->InventoryComponent->WeaponSlot.IsValid())
            {
                Client_ShowNotification(FText::FromString(TEXT("Weapon slot is full!")));

                return;
            }
            PC->InventoryComponent->Server_EquipItem(Item);
        }
        else if ((Item.ItemType == EItemType::Armor))
        {
            if (PC->InventoryComponent->ArmorSlot.IsValid())
            {
                Client_ShowNotification(FText::FromString(TEXT("Armor slot is full!")));
                return;
            }

            PC->InventoryComponent->Server_EquipItem(Item);
        }
        else
        {
            if (!PC->InventoryComponent->HasSpace(Item))
            {
                Client_ShowNotification(FText::FromString(TEXT("Inventory is full!")));
                return;
            }
            PC->InventoryComponent->Server_AddItem(Item);
        }
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), -Item.ItemDataAsset->BuyPrice);
        ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}

// Initial Setup and setting Auth Token
void AM_PlayerController::Server_SetAuthToken_Implementation(const FString &Token)
{
    PlayerAuthToken = Token;
    //	ClientTravel(TEXT("192.168.181.168"), ETravelType::TRAVEL_Absolute);
    LoadPlayerData();
}
void AM_PlayerController::Server_SetupAttributes_Implementation(int32 Level, int32 XP, float Mana, float Health,
                                                                float Coins)
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PlayerChar))
        return;

    FGameplayTag PlayerTag = PlayerChar->PlayerClassTag;

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();

    UE_LOG(LogTemp, Warning, TEXT("Mana: %f"), Mana);

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(PlayerChar->InitStatsEffect, 1, Context);

    float XpMaxLimit = 0.f;
    float MaxMana = 0.f;
    float MaxHealth = 0.f;
    float Energy = 500.f;
    float MaxEnergy = 500.f;

    if (IsValid(PlayerChar->XpScaleTable) &&
        PlayerTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Classes.Warrior")))
    {
        XpMaxLimit = PlayerChar->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Level);
        MaxMana = PlayerChar->XpScaleTable->FindCurve(FName("MaxManaWarrior"), "Mana Lookup")->Eval(Level);
        MaxHealth = PlayerChar->XpScaleTable->FindCurve(FName("MaxHealthWarrior"), "Health Lookup")->Eval(Level);

        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxMana"), MaxMana);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.XpMax"), XpMaxLimit);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxEnergy"), MaxEnergy);
    }
    else if (IsValid(PlayerChar->XpScaleTable) &&
             PlayerTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Classes.Rogue")))
    {
        XpMaxLimit = PlayerChar->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Level);
        MaxMana = PlayerChar->XpScaleTable->FindCurve(FName("MaxManaMage"), "Mana Lookup")->Eval(Level);
        MaxHealth = PlayerChar->XpScaleTable->FindCurve(FName("MaxHealthMage"), "Health Lookup")->Eval(Level);

        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxMana"), MaxMana);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.XpMax"), XpMaxLimit);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            Spec, FGameplayTag::RequestGameplayTag("Data.MaxEnergy"), MaxEnergy);
    }

    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Health"),
                                                                  Health);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Mana"),
                                                                  Mana);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.XP"),
                                                                  XP);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins.Initialize"), Coins);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Level"),
                                                                  Level);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(Spec, FGameplayTag::RequestGameplayTag("Data.Energy"),
                                                                  Energy);

    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

    PlayerChar->SetupAttributes();
}
void AM_PlayerController::Client_OpenQuantityWidget_Implementation(FItemData ItemData, FVector2D SlotPosition,
                                                                   int32 SlotIndex)
{

    if (!IsValid(QuantityWidget))
        return;

    if (!QuantityWidget->bIsOpen)
        QuantityWidget->ShowQuantityPopUp(ItemData, SlotPosition, SlotIndex);
    else
        QuantityWidget->HideQuantityPopUp(ItemData);
}

void AM_PlayerController::Client_ShowOptionsWidget_Implementation()
{
    if (!OptionsWidgetClass)
    {
        return;
    }

    if (!OptionsWidgetInstance)
    {
        OptionsWidgetInstance = CreateWidget<UUserWidget>(this, OptionsWidgetClass);
    }

    if (OptionsWidgetInstance && !OptionsWidgetInstance->IsInViewport())
    {
        OptionsWidgetInstance->AddToViewport();
    }

    SetInputMode(FInputModeGameAndUI());
    SetShowMouseCursor(true);
}

void AM_PlayerController::Client_HideOptionsWidget_Implementation()
{
    if (OptionsWidgetInstance && OptionsWidgetInstance->IsInViewport())
    {
        OptionsWidgetInstance->RemoveFromParent();
    }

    SetInputMode(FInputModeGameOnly());
    SetShowMouseCursor(false);
}

FVector AM_PlayerController::GetCorrectZLocation(FVector TargetLocation)
{
    FVector TraceStart = TargetLocation + FVector(0.f, 0.f, 1000.f);
    FVector TraceEnd = TargetLocation - FVector(0.f, 0.f, 1000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {

        return HitResult.Location + FVector(0.f, 0.f, 90.f);
    }

    return TargetLocation;
}

FVector AM_PlayerController::GetSafeRespawnPoint(FVector DeathLocation)
{
    TArray<AActor *> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AM_SpawnPoints::StaticClass(), SpawnPoints);

    TArray<AActor *> NearbyEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), NearbyEnemies);

    AM_SpawnPoints *BestSpawn = nullptr;
    float BestDistance = MAX_FLT;

    for (AActor *Actor : SpawnPoints)
    {
        AM_SpawnPoints *Spawn = Cast<AM_SpawnPoints>(Actor);
        if (!IsValid(Spawn))
            continue;

        bool bIsSafe = true;
        for (AActor *Enemy : NearbyEnemies)
        {
            if (!IsValid(Enemy))
                continue;
            if (FVector::Dist(Spawn->GetActorLocation(), Enemy->GetActorLocation()) < Spawn->SafeRadius)
            {
                bIsSafe = false;
                break;
            }
        }

        if (!bIsSafe)
            continue;

        float Distance = FVector::Dist(Spawn->GetActorLocation(), DeathLocation);
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestSpawn = Spawn;
        }
    }

    if (IsValid(BestSpawn))
        return BestSpawn->GetActorLocation();

    if (SpawnPoints.Num() > 0)
    {
        AActor *Fallback = SpawnPoints[0];
        if (IsValid(Fallback))
            return Fallback->GetActorLocation();
    }

    return DeathLocation;
}

void AM_PlayerController::PlayTheSound()
{

    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(PlayerChar))
        return;

    if (IsValid(GenericSoundAudioComponent))
    {
        GenericSoundAudioComponent->OnAudioFinished.RemoveDynamic(this, &AM_PlayerController::OnFinishingGenericSound);
        GenericSoundAudioComponent->FadeOut(0.3f, 0.0f);
        GenericSoundAudioComponent = nullptr;
    }

    if (!GetSpotted())
    {
        GenericSoundAudioComponent = nullptr;
        if (GenericGameSounds.IsValidIndex(SoundIndex))
        {

            GenericSoundAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
                this, GenericGameSounds[SoundIndex], PlayerChar->GetActorLocation(), FRotator::ZeroRotator, 0.3f);

            if (GenericSoundAudioComponent)
            {
                GenericSoundAudioComponent->FadeIn(0.3f, 1.0f);
                GenericSoundAudioComponent->OnAudioFinished.AddDynamic(this,
                                                                       &AM_PlayerController::OnFinishingGenericSound);
            }
        }
    }
    else
    {
        GenericSoundAudioComponent = nullptr;
        if (SoundCombat.IsValidIndex(SoundIndex))
        {

            GenericSoundAudioComponent =
                UGameplayStatics::SpawnSoundAtLocation(this, SoundCombat[SoundIndex], PlayerChar->GetActorLocation());

            if (GenericSoundAudioComponent)
            {
                GenericSoundAudioComponent->FadeIn(0.3f, 1.0f);
                GenericSoundAudioComponent->OnAudioFinished.AddDynamic(this,
                                                                       &AM_PlayerController::OnFinishingGenericSound);
            }
        }
    }
}

void AM_PlayerController::OnFinishingGenericSound()
{
    const TArray<USoundBase *> &ActiveSoundArray = GetSpotted() ? SoundCombat : GenericGameSounds;

    if (ActiveSoundArray.Num() > 1)
    {
        int32 NewIndex;
        do
        {
            NewIndex = FMath::RandRange(0, ActiveSoundArray.Num() - 1);
        } while (NewIndex == SoundIndex);

        SoundIndex = NewIndex;
    }
    else if (ActiveSoundArray.Num() == 1)
    {
        SoundIndex = 0;
    }
}

void AM_PlayerController::Client_NotifyUserByEnemyPerception_Implementation(bool bPerceived, AActor *Enemy)
{
    bool bWasSpotted = GetSpotted();

    if (bPerceived)
    {
        EnemiesSpottingMe.AddUnique(Enemy);
    }
    else
    {
        EnemiesSpottingMe.Remove(Enemy);
    }

    bool bIsSpottedNow = GetSpotted();

    if (bWasSpotted == bIsSpottedNow)
        return;

    if (bIsSpottedNow)
    {
        GetWorldTimerManager().ClearTimer(MusicDebounceTimer);
        SoundIndex = 0;
        PlayTheSound();
    }
    else
    {
        GetWorldTimerManager().ClearTimer(MusicDebounceTimer);
        GetWorldTimerManager().SetTimer(
            MusicDebounceTimer,
            [this]() {
                SoundIndex = 0;
                PlayTheSound();
            },
            MusicDebounceDelay, false);
    }
}