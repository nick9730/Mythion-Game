// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_BackendComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Backend/M_BackendSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "DataAsset/CharacterClasses.h"
#include "Dom/JsonObject.h"
#include "GameFramework/Controller.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "M_PlayerController.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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
                    //  PCController->Server_SetupAttributes(Level, XP, Mana, Health, Coins);
                    break;
                }
            }
        }

        if (IsValid(PCController))
        {
            PCController->Server_SetupAttributes(Level, XP, Mana, Health, Coins);
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
                FVector SafeLocation = PCController->GetCorrectZLocation(LastLocation);
                PCController->Server_SetPlayerLocation(SafeLocation);
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
        PCController->LoadInventory();
    }
}
