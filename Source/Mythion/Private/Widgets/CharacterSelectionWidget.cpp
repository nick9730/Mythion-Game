// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterSelectionWidget.h"
#include "Components/Button.h"


#include "DataAsset/CharacterSelectDataAsset.h"



void UCharacterSelectionWidget::OnNextButtonClicked()
	{

	
	
	
}

	void UCharacterSelectionWidget::OnPreviousButtonClicked()
	{


	}

	void UCharacterSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &UCharacterSelectionWidget::OnNextButtonClicked);
	}

	if(PreviousButton)
	{
		PreviousButton->OnClicked.AddDynamic(this, &UCharacterSelectionWidget::OnPreviousButtonClicked);
	}


}


