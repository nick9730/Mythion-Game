// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "M_CommonAbilities.generated.h"

/**
 * 
 */

struct FGameplayTag;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FGrantCommonAbilitiesData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    int32 RequiredLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities ")
    FGameplayTag Tag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TSubclassOf<UGameplayAbility> Ability;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities ")
    int32 InputID;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    FGameplayTag CooldownTag;

 
};

UCLASS()
class MYTHION_API UM_CommonAbilities : public UDataAsset
{
	GENERATED_BODY()
	
public: 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TArray<FGrantCommonAbilitiesData> CommonAbilities;
	
	
};
