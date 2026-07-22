// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_Options.generated.h"

/**
 * 
 */
class UButton;

UCLASS()
class MYTHION_API UM_Options : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UButton* Settings;



	UPROPERTY(meta = (BindWidget))
	UButton* QuitGameButton;

	virtual void NativeConstruct() override;



	UPROPERTY()
	bool bOpenWidget = false;

	UPROPERTY(meta = (BindWidget))
	class USlider* MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	class UComboBoxString* GraphicsQualityDropdown;

	UFUNCTION()
	void OnVolumeChanged(float NewValue);

	UFUNCTION()
	void OnGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnQuitGameClicked();
	
};
