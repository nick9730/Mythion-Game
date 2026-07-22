// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_AbilitiesOfClasses.generated.h"

/**
 * 
 */
class UHorizontalBox;
class UImage;
class UTextBlock;

UCLASS()
class MYTHION_API UM_AbilitiesOfClasses : public UUserWidget
{
	GENERATED_BODY()

public: 

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* WrapperOfAbility;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AbilityName;

	UPROPERTY(meta = (BindWidget))
	UImage* AbilityIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock*  RequiredLevel;	

	UFUNCTION()
	void InitAbilityBox(FName InAbilityName, UTexture2D* InAbilityIcon, int32 InRequiredLevel);
	
};
