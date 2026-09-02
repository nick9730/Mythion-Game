// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/PlayerWidget/Options/M_Options.h"
#include "BackEnd/M_BackendSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_BackendComponent.h"
#include "Sound/SoundClass.h"

void UM_Options::NativeConstruct()
{
    Super::NativeConstruct();

    if (QuitGameButton)
    {
        QuitGameButton->OnClicked.AddDynamic(this, &UM_Options::OnQuitGameClicked);
    }
    if (MasterVolumeSlider)
    {
        MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UM_Options::OnVolumeChanged);
        MasterVolumeSlider->SetValue(1.0f);
    }

    if (GraphicsQualityDropdown)
    {
        GraphicsQualityDropdown->ClearOptions();
        GraphicsQualityDropdown->OnSelectionChanged.AddDynamic(this, &UM_Options::OnGraphicsQualityChanged);
        GraphicsQualityDropdown->AddOption(TEXT("Low"));
        GraphicsQualityDropdown->AddOption(TEXT("Medium"));
        GraphicsQualityDropdown->AddOption(TEXT("High"));
        GraphicsQualityDropdown->AddOption(TEXT("Epic"));
        GraphicsQualityDropdown->SetSelectedOption(TEXT("High"));
    }
}
/*
void UM_Options::OpenOptionsWidget()
{
    bOpenWidget = true;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Options widget opened"));
    }


}

void UM_Options::CloseOptionsWidget()
{
    UM_BackendSubsystem* BackendSubsystem = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();


    AM_PlayerController* PlayerController = Cast<AM_PlayerController>(GetOwningPlayer());


  PlayerController->ClientTravel(TEXT("LoginLayer"), ETravelType::TRAVEL_Absolute);
   BackendSubsystem->Logout();


}
*/

void UM_Options::OnQuitGameClicked()
{
    UM_BackendSubsystem *BackendSubsystem = GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    AM_PlayerController *PlayerController = Cast<AM_PlayerController>(GetOwningPlayer());
    if (!IsValid(BackendSubsystem) || !IsValid(PlayerController))
        return;
    PlayerController->BackendComponent->RequestInventorySave();
    PlayerController->BackendComponent->Client_RequestStatsSave();

    // BackendSubsystem->Logout();
    APlayerCharacter *PlayerChar = Cast<APlayerCharacter>(PlayerController->GetPawn());
    if (IsValid(PlayerChar))
    {
        PlayerChar->Server_DestroyEquippedItems();
    }

    FTimerHandle LogoutDelayHandle;
    GetWorld()->GetTimerManager().SetTimer(
        LogoutDelayHandle,
        [this, BackendSubsystem, PlayerController]() {
            BackendSubsystem->Logout();

            FTimerHandle QuitDelayHandle;
            GetWorld()->GetTimerManager().SetTimer(
                QuitDelayHandle,
                [this, PlayerController]() {
                    UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
                },
                1.0f, false);
        },
        1.5f, false);
}

void UM_Options::OnVolumeChanged(float NewValue)
{
    UGameplayStatics::SetSoundMixClassOverride(
        this, nullptr, USoundClass::StaticClass()->GetDefaultObject<USoundClass>(), NewValue, 1.0f, 0.0f, true);
}

void UM_Options::OnGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    UGameUserSettings *SettingsOfTheGame = GEngine->GetGameUserSettings();
    if (!SettingsOfTheGame)
        return;

    int32 QualityLevel = 2;
    if (SelectedItem == "Low")
        QualityLevel = 0;
    else if (SelectedItem == "Medium")
        QualityLevel = 1;
    else if (SelectedItem == "High")
        QualityLevel = 2;
    else if (SelectedItem == "Epic")
        QualityLevel = 3;

    SettingsOfTheGame->SetOverallScalabilityLevel(QualityLevel);
    SettingsOfTheGame->ApplySettings(false);
    SettingsOfTheGame->SaveSettings();
}