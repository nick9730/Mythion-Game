// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/M_AbilitiesContainer.h"
#include "GameInstances/M_GameInstance.h"
#include "DataAsset/CharacterClasses.h"
#include "Widgets/M_AbilitySlot.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Characters/BaseCharacter.h"
#include  "AbilitySystemBlueprintLibrary.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"	
#include "Components/Widget.h"
#include "Characters/PlayerCharacter.h"
#include "Components/HorizontalBoxSlot.h"
#include "Attributes/M_AttributeSet.h"






void UM_AbilitiesContainer::OnASCReady(UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	APlayerController* PC = GetOwningPlayer();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerChar)) return;

	
	for (const FCharacterClassData& ClassData : PlayerChar->CharacterClassData->CharacterClasses)
	{
		if (ClassData.ClassNameTag.MatchesTagExact(PlayerChar->PlayerClassTag))
		{
			InitializeBar(ASC, ClassData.AbilitiesGrantedByLevel, { "Q", "E", "R", "F" });
			break;
		}
	}

	

}




void UM_AbilitiesContainer::AddAbilitySlot(FGrantAbilitiesDataByLevel AbilityEntry)	
{

	UM_AbilitySlot* AbilitySlot = CreateWidget<UM_AbilitySlot>(GetOwningPlayer(), AbilitySlotClass);
	if (!IsValid(AbilitySlot)) return;

	AbilitySlot->SetAbility(AbilityEntry.Tag, AbilityEntry.CooldownTag, AbilityEntry.IconTag, TEXT(""), BoundASC);

	UHorizontalBoxSlot* BoxSlot = AbilitiesContainer->AddChildToHorizontalBox(AbilitySlot);
	if (BoxSlot)
		BoxSlot->SetPadding(FMargin(0.f, 0.f, 10.f, 0.f));
}

void UM_AbilitiesContainer::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = GetOwningPlayer();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerChar)) return;



	PlayerChar->OnASCInitialized.AddDynamic(this, &UM_AbilitiesContainer::OnASCReady);
	PlayerChar->OnAbilityGranted.AddDynamic(this, &UM_AbilitiesContainer::AddAbilitySlot);

}




void UM_AbilitiesContainer::InitializeBar(UAbilitySystemComponent* ASC, const TArray<FGrantAbilitiesDataByLevel>& Abilities, const TArray<FString>& Keybinds)
{
	if (!IsValid(ASC) || !AbilitiesContainer || !AbilitySlotClass) return;

	BoundASC = ASC;
	AbilitiesContainer->ClearChildren();

	float CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());

	for (int32 i = 0; i < Abilities.Num(); i++)
	{
		const FGrantAbilitiesDataByLevel& AbilityEntry = Abilities[i];

		if (AbilityEntry.RequiredLevel > FMath::FloorToInt(CurrentLevel)) continue;

		UM_AbilitySlot* AbilitySlot = CreateWidget<UM_AbilitySlot>(GetOwningPlayer(), AbilitySlotClass);
		if (!IsValid(AbilitySlot)) continue;

		FString Keybind = Keybinds.IsValidIndex(i) ? Keybinds[i] : TEXT("");

		AbilitySlot->SetAbility(
			AbilityEntry.Tag,
			AbilityEntry.CooldownTag,
			AbilityEntry.IconTag,
			Keybind,
			ASC
		);

		UHorizontalBoxSlot* BoxSlot = AbilitiesContainer->AddChildToHorizontalBox(AbilitySlot);
		if (BoxSlot)
		{
			BoxSlot->SetPadding(FMargin(40.f, 0.f, 40.f, 0.f));
		}
	}
}




