// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/M_Enemy_Verticals.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/Enemy.h"

void UM_Enemy_Verticals::InitializeWidget(UAbilitySystemComponent* ASC, const FString& EnemyName, int32 EnemyLevel)
{
    if (!IsValid(ASC)) return;

    if (NameText)
        NameText->SetText(FText::FromString(EnemyName));

    if (LevelText)
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("Lv. %d"), EnemyLevel)));




    BindToASC(ASC);
}

void UM_Enemy_Verticals::SetOwningEnemy(AEnemy* Enemy)
{
    OwningEnemy = Enemy;
}

void UM_Enemy_Verticals::NativeDestruct()
{
    UnbindFromASC();
    Super::NativeDestruct();
}

void UM_Enemy_Verticals::NativeConstruct()
{
    Super::NativeConstruct();
  
    if (!IsValid(OwningEnemy)) return;

    UAbilitySystemComponent* ASC = OwningEnemy->GetAbilitySystemComponent();
    if (!IsValid(ASC)) return;

    int32 Level = FMath::FloorToInt(
        ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute()));

    InitializeWidget(ASC, OwningEnemy->EnemyDisplayName, Level);
}

void UM_Enemy_Verticals::BindToASC(UAbilitySystemComponent* ASC)
{
    BoundASC = ASC;

    CurrentHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
    CurrentMaxHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());

    HealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(
        UM_AttributeSet::GetHealthAttribute()).AddUObject(this, &UM_Enemy_Verticals::OnHealthChanged);
    MaxHealthHandle = ASC->GetGameplayAttributeValueChangeDelegate(
        UM_AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UM_Enemy_Verticals::OnMaxHealthChanged);

    UpdateHealthBar();
}

void UM_Enemy_Verticals::UnbindFromASC()
{
    if (!IsValid(BoundASC)) return;

    BoundASC->GetGameplayAttributeValueChangeDelegate(
        UM_AttributeSet::GetHealthAttribute()).Remove(HealthHandle);
    BoundASC->GetGameplayAttributeValueChangeDelegate(
        UM_AttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthHandle);

    BoundASC = nullptr;
}

void UM_Enemy_Verticals::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentHealth = Data.NewValue;
    UpdateHealthBar();
}

void UM_Enemy_Verticals::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentMaxHealth = Data.NewValue;
    UpdateHealthBar();
}

void UM_Enemy_Verticals::UpdateHealthBar()
{
    if (HealthBar && CurrentMaxHealth > 0.f)
        HealthBar->SetPercent(FMath::Clamp(CurrentHealth / CurrentMaxHealth, 0.f, 1.f));
}