// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/PlayerWidget/StatsWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"

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
	UAbilitySystemComponent* ASC = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
 
    if (!IsValid(ASC)) return;
     BindToASC(ASC);
}

void UStatsWidget::BindToASC(UAbilitySystemComponent* ASC)
{
    BoundASC = ASC;

    UE_LOG(LogTemp, Warning, TEXT("BindToASC called"));
    BoundASC = ASC;

    CurrentHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    UE_LOG(LogTemp, Warning, TEXT("Initial Health: %f"), CurrentHealth);

    UE_LOG(LogTemp, Warning, TEXT("At bind time - Health: %f MaxHealth: %f"), CurrentHealth, CurrentMaxHealth);

 
    CurrentHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    CurrentMaxHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
    CurrentMana = ASC->GetNumericAttribute(UM_AttributeSet::GetManaAttribute());
    CurrentMaxMana = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxManaAttribute());
    CurrentXP = ASC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());
    CurrentXPMax = ASC->GetNumericAttribute(UM_AttributeSet::GetXpMaxAttribute());
    UE_LOG(LogTemp, Warning, TEXT("Initial XPMax: %f"), CurrentXPMax);

    CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());

    // Bind delegates
    HealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetHealthAttribute()).AddUObject(this, &UStatsWidget::OnHealthChanged);
    MaxHealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UStatsWidget::OnMaxHealthChanged);
    ManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetManaAttribute()).AddUObject(this, &UStatsWidget::OnManaChanged);
    MaxManaHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetMaxManaAttribute()).AddUObject(this, &UStatsWidget::OnMaxManaChanged);
    XPHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetXpAttribute()).AddUObject(this, &UStatsWidget::OnXPChanged);
    XPMaxHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetXpMaxAttribute()).AddUObject(this, &UStatsWidget::OnXPMaxChanged);    LevelHandle = ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetLevelAttribute()).AddUObject(this, &UStatsWidget::OnLevelChanged);
    
   
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

void UStatsWidget::UpdateHealthBar()
{
    UE_LOG(LogTemp, Warning, TEXT("UpdateHealthBar - Health: %f MaxHealth: %f Percent: %f"),
        CurrentHealth, CurrentMaxHealth, CurrentHealth / CurrentMaxHealth);
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





