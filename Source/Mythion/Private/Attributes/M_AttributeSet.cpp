// Fill out your copyright notice in the Description page of Project Settings.


#include "Attributes/M_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "M_PlayerController.h"
#include "Characters/PlayerCharacter.h"
#include "Engine/Engine.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include <Kismet/GameplayStatics.h>



UM_AttributeSet::UM_AttributeSet()
{

 InitLevel(1.f);

}

void UM_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, MagicResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Xp, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, XpMax, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_AttributeSet, Coins, COND_None, REPNOTIFY_Always);
}

void UM_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{

		if (GetMaxHealth() > 0.f)
		{
			SetMaxHealth(FMath::Max(GetMaxHealth(), 1.f));
			SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		}
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		if (GetMaxMana() > 0.f)
		{
			SetMaxMana(FMath::Max(GetMaxMana(), 1.f));
			SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
		}
	}

	/*
	*/


	if (Data.EvaluatedData.Attribute == GetXpAttribute() || Data.EvaluatedData.Attribute == GetXpMaxAttribute()) 
	{
		APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningActor());
		if (IsValid(Character) && IsValid(Character->XpScaleTable)) {
			CurrentLevel = GetOwningAbilitySystemComponent()->GetNumericAttributeBase(GetLevelAttribute());
			XpMaxLimit = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(CurrentLevel);
			SetXpMax(XpMaxLimit);
				}		
	}

	if (Data.EvaluatedData.Attribute == GetLevelAttribute()) {
;
	
		
	}


			if (!bAttributesInitialized)
			{

				bAttributesInitialized = true;

				APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningActor());
				if (IsValid(Character) && IsValid(Character->XpScaleTable))
				{
					float Levele = FMath::Max(GetLevel(), 1.f);
					XpMaxLimit = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(Levele);
					SetXpMax(XpMaxLimit);
				}

				OnAttributesInitialized.Broadcast();
			}
	
}

void UM_AttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{

	Super::PreAttributeBaseChange(Attribute, NewValue);

	APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningActor());
	if (IsValid(Character) && IsValid(Character->XpScaleTable)) {
		CurrentLevel = GetOwningAbilitySystemComponent()->GetNumericAttributeBase(GetLevelAttribute());
		XpMaxLimit = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(CurrentLevel);


	if (Attribute == GetXpAttribute())
	{
			while (NewValue >= XpMaxLimit)
			{

				NewValue = NewValue - XpMaxLimit;

				APlayerCharacter* CharacterE = Cast<APlayerCharacter>(GetOwningActor());
				UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CharacterE->LevelUpEffect, 1.f, Context);
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);

				CurrentLevel = GetOwningAbilitySystemComponent()->GetNumericAttributeBase(GetLevelAttribute());

				FGameplayEffectSpecHandle SpecLevelUp = ASC->MakeOutgoingSpec(CharacterE->LevelUpStats, CurrentLevel, Context);
				ASC->ApplyGameplayEffectSpecToSelf(*SpecLevelUp.Data);
				OnLevelUp.Broadcast(CurrentLevel);
				UE_LOG(LogTemp, Warning, TEXT("Broadcasting OnLevelUp: %f"), CurrentLevel);
		
				XpMaxLimit = Character->XpScaleTable->FindCurve(FName("XpMax"), "XP Lookup")->Eval(CurrentLevel);
			}
		}


	}
}






void UM_AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (!IsValid(ASC)) return;
		GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MaxHealth, OldValue);
	
}

void UM_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Health, OldValue);
}

void UM_AttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Mana, OldValue);
}

void UM_AttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MaxMana, OldValue);
}

void UM_AttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Armor, OldValue);
}

void UM_AttributeSet::OnRep_MagicResistance(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, MagicResistance, OldValue);
}

void UM_AttributeSet::OnRep_Xp(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Xp, OldValue);
}

void UM_AttributeSet::OnRep_XpMax(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, XpMax, OldValue);
}

void UM_AttributeSet::OnRep_Level(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Level, OldValue);
}

void UM_AttributeSet::OnRep_Coins(const FGameplayAttributeData& OldValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!IsValid(ASC)) return;
	GAMEPLAYATTRIBUTE_REPNOTIFY(UM_AttributeSet, Coins, OldValue);
}
