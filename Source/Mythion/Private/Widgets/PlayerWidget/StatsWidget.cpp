// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/PlayerWidget/StatsWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Characters/Enemy.h"
#include "Chat/M_ChatWidget.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "M_QuestsInfos.h"


void UStatsWidget::InitializeWidget(UAbilitySystemComponent* ASC)
{
    if (!IsValid(ASC)) return;
    BindToASC(ASC);
}

void UStatsWidget::NativeDestruct()
{
    Super::NativeDestruct();

}

void UStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();
//	UAbilitySystemComponent* ASC = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
 
  //  if (!IsValid(ASC)) return;


    APlayerController* PC = GetOwningPlayer();
    if (!IsValid(PC)) return;

    APlayerState* PS = PC->GetPlayerState<APlayerState>();
    if (!IsValid(PS)) return;

    UAbilitySystemComponent* ASC = PS->FindComponentByClass<UAbilitySystemComponent>();
    if (!IsValid(ASC)) return;

    if (QuestInfoWidget)
        QuestInfoWidget->SetVisibility(ESlateVisibility::Collapsed);

    TryBindASC();
}

void UStatsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    
}

FVector2d UStatsWidget::GetMiniMapPosition(APlayerCharacter* Player, AActor* Target, float MinimapSize, float CaptureRange)
{
    FVector PlayerLoc = Player->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();

    FVector Delta = TargetLoc - PlayerLoc;

    float X = (Delta.X / CaptureRange) * (MinimapSize / 2.f) + (MinimapSize / 2.f);
    float Y = (-Delta.Y / CaptureRange) * (MinimapSize / 2.f) + (MinimapSize / 2.f);

    return FVector2D(X, Y);
}

void UStatsWidget::TryBindASC()
{
    APlayerController* PC = GetOwningPlayer();
    APlayerState* PS = PC ? PC->PlayerState : nullptr;
    UAbilitySystemComponent* ASC = PS ? PS->FindComponentByClass<UAbilitySystemComponent>() : nullptr;

    if (ASC)
    {
        float TestMaxEnergy = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxEnergyAttribute());
        if (TestMaxEnergy <= 0.f)
        {
            
            GetWorld()->GetTimerManager().SetTimer(
                BindASCTimerHandle, this, &UStatsWidget::TryBindASC, 0.2f, false
            );
            return;
        }

        BindToASC(ASC);
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimer(
            BindASCTimerHandle, this, &UStatsWidget::TryBindASC, 0.2f, false
        );
    }
}

void UStatsWidget::BindToASC(UAbilitySystemComponent* ASC)
{
    BoundASC = ASC;
 
    CurrentHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    CurrentMaxHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
    CurrentMana = ASC->GetNumericAttribute(UM_AttributeSet::GetManaAttribute());
    CurrentMaxMana = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxManaAttribute());
    CurrentXP = ASC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());
    CurrentXPMax = ASC->GetNumericAttribute(UM_AttributeSet::GetXpMaxAttribute());
    CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
    CurrentEnergy = ASC->GetNumericAttribute(UM_AttributeSet::GetEnergyAttribute());
    CurrentMaxEnergy = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxEnergyAttribute());


    // Bind delegates
    HealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetHealthAttribute()).AddUObject(this, &UStatsWidget::OnHealthChanged);
    MaxHealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UStatsWidget::OnMaxHealthChanged);
    ManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetManaAttribute()).AddUObject(this, &UStatsWidget::OnManaChanged);
    MaxManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMaxManaAttribute()).AddUObject(this, &UStatsWidget::OnMaxManaChanged);
    XPHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetXpAttribute()).AddUObject(this, &UStatsWidget::OnXPChanged);
    XPMaxHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetXpMaxAttribute()).AddUObject(this, &UStatsWidget::OnXPMaxChanged);  
    LevelHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetLevelAttribute()).AddUObject(this, &UStatsWidget::OnLevelChanged);
    EnergyHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetEnergyAttribute()).AddUObject(this, &UStatsWidget::OnEnergyChanged);
    MaxEnergyHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMaxEnergyAttribute()).AddUObject(this, &UStatsWidget::OnMaxEnergyChanged);

   
    UpdateHealthBar();
    UpdateManaBar();
    UpdateXPBar();

    if (LevelText)
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), FMath::FloorToInt(CurrentLevel))));
}

void UStatsWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentHealth = Data.NewValue;
    UpdateHealthBar();
}

void UStatsWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentMaxHealth = Data.NewValue;
    UpdateHealthBar();
}

void UStatsWidget::OnManaChanged(const FOnAttributeChangeData& Data)
{
    CurrentMana = Data.NewValue;
    UpdateManaBar();
}

void UStatsWidget::OnMaxManaChanged(const FOnAttributeChangeData& Data)
{
    CurrentMaxMana = Data.NewValue;
    UpdateManaBar();
}

void UStatsWidget::OnXPChanged(const FOnAttributeChangeData& Data)
{
    CurrentXP = Data.NewValue;
    UpdateXPBar();
}


void UStatsWidget::OnXPMaxChanged(const FOnAttributeChangeData& Data)
{
	CurrentXPMax = Data.NewValue;
    UpdateXPBar();
}

void UStatsWidget::OnLevelChanged(const FOnAttributeChangeData& Data)
{
    CurrentLevel = Data.NewValue;
    if (LevelText)
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), FMath::FloorToInt(CurrentLevel))));
}



void UStatsWidget::OnEnergyChanged(const FOnAttributeChangeData& Data)
{
    CurrentEnergy = Data.NewValue;
    UpdateEnergyBar();
}
void UStatsWidget::OnMaxEnergyChanged(const FOnAttributeChangeData& Data)
{
    CurrentMaxEnergy = Data.NewValue;
    UpdateEnergyBar();
}


void UStatsWidget::UpdateHealthBar()
{
        
    if (HealthBar && CurrentMaxHealth > 0.f)
        HealthBar->SetPercent(FMath::Clamp(CurrentHealth / CurrentMaxHealth, 0.f, 1.f));

    if (HealthText)
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
            FMath::FloorToInt(CurrentHealth), FMath::FloorToInt(CurrentMaxHealth))));
}

void UStatsWidget::UpdateManaBar()
{
    if (ManaBar && CurrentMaxMana > 0.f)
        ManaBar->SetPercent(FMath::Clamp(CurrentMana / CurrentMaxMana, 0.f, 1.f));

    if (ManaText)
        ManaText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
            FMath::FloorToInt(CurrentMana), FMath::FloorToInt(CurrentMaxMana))));
}

void UStatsWidget::UpdateXPBar()
{
    if (XPBar && CurrentXPMax > 0.f)
        XPBar->SetPercent(FMath::Clamp(CurrentXP / CurrentXPMax, 0.f, 1.f));

    if (XPText)
        XPText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
            FMath::FloorToInt(CurrentXP), FMath::FloorToInt(CurrentXPMax))));
}


void UStatsWidget::UpdateEnergyBar()
{
    if (EnergyBar && CurrentMaxEnergy > 0.f)
        EnergyBar->SetPercent(FMath::Clamp(CurrentEnergy / CurrentMaxEnergy, 0.f, 1.f));

    if (EnergyText)
        EnergyText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
            FMath::FloorToInt(CurrentEnergy), FMath::FloorToInt(CurrentMaxEnergy))));
}


void UStatsWidget::FocusChatInput()
{
    if (ChatWidget)
    {
        ChatWidget->FocusInput();
    }
}
void UStatsWidget::UnFocusChatInput()
{
    if (ChatWidget)
    {
        ChatWidget->UnfocusInput();
    }
}