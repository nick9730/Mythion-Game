// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/XpWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

/*

void UXpWidget::UpdateXpDisplay()
{

    APlayerCharacter* OwningActor = Cast<APlayerCharacter>(GetOwningPlayerPawn());


    if (!XpBar || !XpPoints || !OwningActor) return;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningActor);


    if (ASC)
    {
        float MaxXp = ASC->GetNumericAttributeChecked(UM_AttributeSet::GetXpMaxAttribute());
        float CurrentXp = ASC->GetNumericAttributeChecked(UM_AttributeSet::GetXpAttribute());
		float CurrentLevel = ASC->GetNumericAttributeChecked(UM_AttributeSet::GetLevelAttribute());
        if (GEngine) {
            UE_LOG(LogTemp, Warning, TEXT("Current Level in widget is: %f"), CurrentLevel);

        }
        if (GEngine) {
            UE_LOG(LogTemp, Warning, TEXT("Current MaxXp in widget is: %f"), MaxXp);

        }

        if (MaxXp > 0.f)
        {
            XpBar->SetPercent(CurrentXp / MaxXp);
        }

        FText XpText = FText::Format(
            FText::FromString("{0} / {1}"),
            FText::AsNumber(FMath::FloorToInt(CurrentXp)),
            FText::AsNumber(FMath::FloorToInt(MaxXp))
        );

        FText LevelText = FText::Format(
            FText::FromString("{0}"),
            FText::AsNumber(FMath::FloorToInt(CurrentLevel))
           
        );
		Level->SetText(LevelText);
        XpPoints->SetText(XpText);
    }
}

*/