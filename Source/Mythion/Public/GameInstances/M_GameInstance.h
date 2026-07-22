// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DataAsset/CharacterSelectDataAsset.h"
#include "GameplayTagContainer.h"
#include "M_GameInstance.generated.h"


class UCharacterClasses;
	

UCLASS()
class MYTHION_API UM_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:	

    UM_GameInstance();


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
    TSoftObjectPtr<UCharacterClasses> SelectedClassData;

    UPROPERTY(BlueprintReadWrite, Category = "Mythion | Character Selection")
    FGameplayTag SelectedClassTag;

    // Backend Auth
    UPROPERTY(BlueprintReadWrite, Category = "Mythion | Backend")
    FString AuthToken;

    UPROPERTY(BlueprintReadWrite, Category = "Mythion | Backend")
    FString LoggedInUsername;

    UPROPERTY(BlueprintReadWrite, Category = "Mythion | Backend")
    bool bIsLoggedIn = false;

    // Clear token on logout
    UFUNCTION(BlueprintCallable, Category = "Mythion | Backend")
    void Logout();

	
};
