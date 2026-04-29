// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAsset/CharacterClasses.h"
#include "M_AbilitiesContainer.generated.h"


class UCharacterClasses;
class UPanelWidget;
class UM_AbilitySlot;
struct FGameplayTag;
class UTexture2D;
class UHorizontalBox;
class UAbilitySystemComponent;
struct FTimerHandle;


UCLASS()
class MYTHION_API UM_AbilitiesContainer : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UHorizontalBox* AbilitiesContainer;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class UM_AbilitySlot> AbilitySlotClass;

	UPROPERTY()
    FTimerHandle BindTimer;

    UPROPERTY()
    UAbilitySystemComponent* BoundASC;

    UFUNCTION()
    void AddAbilitySlot(FGrantAbilitiesDataByLevel AbilityEntry);

  UFUNCTION()
    void OnASCReady(UAbilitySystemComponent* ASC, UAttributeSet* AS);
    
    UFUNCTION()
    void InitializeBar(UAbilitySystemComponent* ASC, const TArray<FGrantAbilitiesDataByLevel>& Abilities, const TArray<FString>& Keybinds);



protected:
    virtual void NativeConstruct() override;
};

