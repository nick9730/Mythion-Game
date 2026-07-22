// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/M_AbilitiesOfClasses.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Engine/Texture2D.h"



void UM_AbilitiesOfClasses::InitAbilityBox(FName InAbilityName, UTexture2D* InAbilityIcon, int32 InRequiredLevel)
{
    if (AbilityName)
    {
        AbilityName->SetText(FText::FromName(InAbilityName));
    }

    if (AbilityIcon && InAbilityIcon)
    {
        AbilityIcon->SetBrushFromTexture(InAbilityIcon);
    }

    if (RequiredLevel)
    {
       RequiredLevel->SetText(FText::AsNumber(InRequiredLevel));
    }
}


