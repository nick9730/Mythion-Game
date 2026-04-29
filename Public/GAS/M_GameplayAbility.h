// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "M_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API UM_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mythion | Debug")
	bool bDrawDebugs = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythion | UI")
	bool bShouldShowInAbilityBar = false;


	
};
