#include "M_PlayerController.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Backend/M_BackendSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "Chat/M_ChatSubSystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Widgets/M_AbilitiesContainer.h"
#include "Widgets/PlayerWidget/StatsWidget.h"

#include "PlayerController/Components/M_BackendComponent.h"
#include "PlayerController/Components/M_InteractionTraceComponent.h"
#include "PlayerController/Components/M_InventoryActionsComponent.h"
#include "PlayerController/Components/M_MusicComponent.h"
#include "PlayerController/Components/M_QuestUIComponent.h"
#include "PlayerController/Components/M_RespawnComponent.h"
#include "PlayerController/Components/M_ShopComponent.h"
#include "PlayerController/Components/M_UIComponent.h"

// Init

AM_PlayerController::AM_PlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    MusicComponent = CreateDefaultSubobject<UM_MusicComponent>(TEXT("MusicComponent"));
    BackendComponent = CreateDefaultSubobject<UM_BackendComponent>(TEXT("BackendComponent"));
    InventoryActionsComponent = CreateDefaultSubobject<UM_InventoryActionsComponent>(TEXT("InventoryActionsComponent"));
    RespawnComponent = CreateDefaultSubobject<UM_RespawnComponent>(TEXT("RespawnComponent"));
    QuestUIComponent = CreateDefaultSubobject<UM_QuestUIComponent>(TEXT("QuestUIComponent"));
    InteractionTraceComponent = CreateDefaultSubobject<UM_InteractionTraceComponent>(TEXT("InteractionTraceComponent"));
    ShopComponent = CreateDefaultSubobject<UM_ShopComponent>(TEXT("ShopComponent"));
    UIComponent = CreateDefaultSubobject<UM_UIComponent>(TEXT("UIComponent"));
}

void AM_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    APlayerCharacter *PC = Cast<APlayerCharacter>(GetPawn());
    FString MapName = GetWorld()->GetMapName();

    if (MapName.Contains("Character_Selection"))
    {
        if (IsValid(PC))
        {
            PC->GetCharacterMovement()->DisableMovement();
            PC->GetCharacterMovement()->StopMovementImmediately();
        }
        SetInputMode(FInputModeUIOnly());
        bShowMouseCursor = true;
    }

    if (IsValid(PC))
    {
        PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
        PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);
    }

    if (IsLocalController() && !MapName.Contains("Character_Selection") && !MapName.Contains("LoginLayer"))
        ShowLoadingScreen();

    MusicComponent->PlayTheSound();

    bStatsLoaded = false;
    bInventoryLoaded = false;
    bQuestsLoaded = false;

    BackendComponent->PlayerAuthToken = PlayerAuthToken;
    BackendComponent->LoadPlayerData();

    if (IsLocalController())
    {
        if (UGameInstance *GI = GetGameInstance())
        {
            if (UM_ChatSubSystem *ChatSub = GI->GetSubsystem<UM_ChatSubSystem>())
            {
                ChatSub->ConnectToChat();
            }
        }

        BackendComponent->LoadPlayerQuests();
        BackendComponent->StartInventorySaveTimer();
        BackendComponent->StartStatsSaveTimer();
    }
}

void AM_PlayerController::Tick(float DeltaSeconds)
{
    if (IsValid(InteractionTraceComponent))
        InteractionTraceComponent->TraceForItem();
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
                Subsystem->AddMappingContext(Context, 0);
        }
    }

    UEnhancedInputComponent *EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!IsValid(EIC))
        return;

    EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Move);
    EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Look);
    EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AM_PlayerController::Jump);
    EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AM_PlayerController::StopJumping);
    EIC->BindAction(InventoryAction, ETriggerEvent::Started, UIComponent.Get(), &UM_UIComponent::ToggleInventory);
    EIC->BindAction(InteractAction, ETriggerEvent::Started, InteractionTraceComponent.Get(),
                    &UM_InteractionTraceComponent::TryInteract);
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
        BackendComponent->PlayerAuthToken = PlayerAuthToken;
    }

    PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
    PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);
    PC->InventoryComponent->OnInventoryFull.AddDynamic(UIComponent, &UM_UIComponent::OnInventoryFull);

    UAbilitySystemComponent *ASC = PC->GetAbilitySystemComponent();
    if (IsValid(ASC))
        OnASCReady(ASC, PC->GetAttributeSet());

    if (IsLocalController())
    {
        if (IsValid(UIComponent))
            UIComponent->TryInitInventory();
    }
}

void AM_PlayerController::OnASCReady(UAbilitySystemComponent *ASC, UAttributeSet *AS)
{
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

// Loading screen

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
}

void AM_PlayerController::CheckAllLoaded()
{
    FString MapName = GetWorld()->GetMapName();

    if (bStatsLoaded && bInventoryLoaded && bQuestsLoaded && !MapName.Contains("Character_Selection") &&
        !MapName.Contains("LoginLayer") && bOnASCReadyForAbilities)
        HideLoadingScreen();
}

// Input actions

void AM_PlayerController::Move(const FInputActionValue &Value)
{
    APlayerCharacter *Char = Cast<APlayerCharacter>(GetPawn());
    if (!IsValid(Char))
        return;

    UAbilitySystemComponent *ASC = Char->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Attacking")))
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
    if (IsValid(GetCharacter()))
        GetCharacter()->Jump();
}

void AM_PlayerController::StopJumping()
{
    if (IsValid(GetCharacter()))
        GetCharacter()->StopJumping();
}

// Backend / attributes

void AM_PlayerController::Server_SetAuthToken_Implementation(const FString &Token)
{
    PlayerAuthToken = Token;
    BackendComponent->LoadPlayerData();
}

void AM_PlayerController::Server_SetupAttributes_Implementation(int32 Level, int32 XP, float Mana, float Health,
                                                                float Coins, int32 Armor, int32 MagicResist)
{
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(GetPawn());

    if (!IsValid(PlayerChar))
    {
        FTimerHandle RetryTimer;
        GetWorld()->GetTimerManager().SetTimer(
            RetryTimer,
            [this, Level, XP, Mana, Health, Coins, Armor, MagicResist]() {
                Server_SetupAttributes(Level, XP, Mana, Health, Coins, Armor, MagicResist);
            },
            0.2f, false);
        return;
    }

    UAbilitySystemComponent *ASC = PlayerChar->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    float XpMaxLimit = 0.f;
    float MaxMana = 0.f;
    float MaxHealth = 0.f;
    const float Energy = 500.f;
    const float MaxEnergy = 500.f;

    UM_AttributeSet::CalculateClassStats(PlayerChar->PlayerClassTag, PlayerChar->XpScaleTable, Level, XpMaxLimit,
                                         MaxMana, MaxHealth);

    UM_AttributeSet::ApplyStatsFromBackend(ASC, PlayerChar->InitStatsEffect, MaxHealth, MaxMana, Level, XP, XpMaxLimit,
                                           Coins, Armor, MagicResist, Energy, MaxEnergy, Health, Mana);

    PlayerChar->SetupAttributes();
}

// Misc

void AM_PlayerController::Client_NotifyUserByEnemyPerception_Implementation(bool bPerceived, AActor *Enemy)
{
    MusicComponent->NotifyEnemyPerception(bPerceived, Enemy);
}

void AM_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}