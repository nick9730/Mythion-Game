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
#include "M_PlayerController.h"






void UM_AbilitiesContainer::OnASCReady(UAbilitySystemComponent* ASC, UAttributeSet* AS)
{

	FString MapName = GetWorld()->GetMapName();
	if (!MapName.Contains("MythionMap") && !MapName.Contains("Untitledeeee")) return;
	APlayerController* PC = GetOwningPlayer();
	
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	AM_PlayerController* PlayerController = Cast<AM_PlayerController>(GetOwningPlayer());
	if (!IsValid(PlayerChar)) return;
	if (!IsValid(PlayerController)) return; 
	

	for (const FCharacterClassData& ClassData : PlayerChar->CharacterClassData->CharacterClasses)
	{
		if (ClassData.ClassNameTag.MatchesTagExact(PlayerChar->PlayerClassTag))
		{
			InitializeBar(ASC, ClassData.AbilitiesGrantedByLevel, { "E", "E", "R", "F" });
			PlayerController->bOnASCReadyForAbilities = true;
			break;
		}
	}

	

}




void UM_AbilitiesContainer::AddAbilitySlot(FGrantAbilitiesDataByLevel AbilityEntry)	
{
	FString MapName = GetWorld()->GetMapName();
	if (!MapName.Contains("MythionMap") && !MapName.Contains("Untitledeeee")) return;


	if (AbilityEntry.bIsPassive) return;

	if (AbilitiesContainer)
	{
		int32 Count = AbilitiesContainer->GetChildrenCount();
		for (int32 i = 0; i < Count; i++)
		{
			UWidget* ChildWidget = AbilitiesContainer->GetChildAt(i);
			UM_AbilitySlot* ExistingSlot = Cast<UM_AbilitySlot>(ChildWidget);

		
			if (ExistingSlot && ExistingSlot->AbilityTag == AbilityEntry.Tag)
			{
				return;
			}
		}
	}
	UM_AbilitySlot* AbilitySlot = CreateWidget<UM_AbilitySlot>(GetOwningPlayer(), AbilitySlotClass);
	if (!IsValid(AbilitySlot)) return;

	UAbilitySystemComponent* ASCToUse = IsValid(BoundASC) ? BoundASC :
		Cast<APlayerCharacter>(GetOwningPlayer()->GetPawn())->GetAbilitySystemComponent();


	AbilitySlot->SetAbility(AbilityEntry.Tag, AbilityEntry.CooldownTag, AbilityEntry.IconTag, AbilityEntry.KeyOfButtonAbility, ASCToUse);

	UHorizontalBoxSlot* BoxSlot = AbilitiesContainer->AddChildToHorizontalBox(AbilitySlot);
	if (BoxSlot) {
		BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		BoxSlot->SetPadding(FMargin(40.f, 0.f, 40.f, 0.f));

	}
}

void UM_AbilitiesContainer::NativeConstruct()
{
	Super::NativeConstruct();

	FString MapName = GetWorld()->GetMapName();


	APlayerController* PC = GetOwningPlayer();
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn());
	if (!IsValid(PlayerChar)) return;

	UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();

	if (IsValid(ASC))
	{
	
		OnASCReady(ASC, PlayerChar->GetAttributeSet());
	}

	PlayerChar->OnASCInitialized.AddDynamic(this, &UM_AbilitiesContainer::OnASCReady);
	PlayerChar->OnAbilityGranted.AddDynamic(this, &UM_AbilitiesContainer::AddAbilitySlot);
	};
	





void UM_AbilitiesContainer::InitializeBar(UAbilitySystemComponent* ASC, const TArray<FGrantAbilitiesDataByLevel>& Abilities, const TArray<FString>& Keybinds)
{
	if (!IsValid(ASC) || !AbilitiesContainer || !AbilitySlotClass) return;



	BoundASC = ASC;
	AbilitiesContainer->ClearChildren();
	AM_PlayerController* PC = Cast<AM_PlayerController>(GetOwningPlayer());
 
		if (IsValid(PC))
		{
			
				PC->HideLoadingScreen();
		}
	

	float CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());


	FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains("MythionMap") || MapName.Contains("Untitledeeee"))
	{
		for (int32 i = 0; i < Abilities.Num(); i++)
		{
			const FGrantAbilitiesDataByLevel& AbilityEntry = Abilities[i];

			if (AbilityEntry.RequiredLevel > FMath::FloorToInt(CurrentLevel)) continue;
			if (AbilityEntry.bIsPassive) continue;

			UM_AbilitySlot* AbilitySlot = CreateWidget<UM_AbilitySlot>(GetOwningPlayer(), AbilitySlotClass);
			if (!IsValid(AbilitySlot)) continue;



			AbilitySlot->SetAbility(
				AbilityEntry.Tag,
				AbilityEntry.CooldownTag,
				AbilityEntry.IconTag,
				AbilityEntry.KeyOfButtonAbility,
				ASC
			);

			UHorizontalBoxSlot* BoxSlot = AbilitiesContainer->AddChildToHorizontalBox(AbilitySlot);
			if (BoxSlot)
			{
				BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				BoxSlot->SetPadding(FMargin(40.f, 0.f, 40.f, 0.f));
			}
		}
	}
}




