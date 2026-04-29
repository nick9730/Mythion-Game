// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/M_AbilitySlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"


void UM_AbilitySlot::SetAbility(FGameplayTag Tag, FGameplayTag Cooldown, UTexture2D* Icon, FString Keybind, UAbilitySystemComponent* ASC)
{
    AbilityTag = Tag;
    CooldownTag = Cooldown;
    BoundASC = ASC;

    if (AbilityIcon && Icon)
        AbilityIcon->SetBrushFromTexture(Icon);

    if (KeybindText)
        KeybindText->SetText(FText::FromString(Keybind));

    if (CooldownBar)
        CooldownBar->SetPercent(0.f);
}

void UM_AbilitySlot::UpdateCooldown()
{


    if (!IsValid(BoundASC) || !CooldownTag.IsValid()) return;


    FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
        FGameplayTagContainer(CooldownTag)
    );

    TArray<TPair<float, float>> Times = BoundASC->GetActiveEffectsTimeRemainingAndDuration(Query);

    
    if (Times.Num() > 0 && Times[0].Value > 0.f)
    {
        float Remaining = Times[0].Key;
        float Duration = Times[0].Value;
        float Percent = FMath::Clamp(Remaining / Duration, 0.f, 1.f);

        if (CooldownBar)
            CooldownBar->SetPercent(Percent);
        if (CooldownText)
            CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
    }
    else
    {
        if (CooldownBar)  CooldownBar->SetPercent(0.f);
        if (CooldownText) CooldownText->SetText(FText::GetEmpty());
    }


}

void UM_AbilitySlot::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateCooldown();
}
