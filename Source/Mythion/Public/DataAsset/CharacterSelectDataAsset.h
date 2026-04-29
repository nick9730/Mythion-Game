// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Containers/Map.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagContainer.h"
#include "CharacterSelectDataAsset.generated.h"


class UTexture2D;
class UAnimInstance;
class USkeletalMesh;
class UAnimMontage;
struct FGameplayTag;



USTRUCT(BlueprintType)
struct FCharacterChoiceData
{
	GENERATED_BODY()

	

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select")
	FGameplayTag CharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
	USkeletalMesh* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class")
	TSubclassOf<UAnimInstance> AnimBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats");
	TMap<FName, float> CharacterStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Stats");
	TMap<FName, float> CharacterStatsBase;

};

UCLASS()
class MYTHION_API UCharacterSelectDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Classes")
	TArray<struct FCharacterChoiceData> CharacterChoices;





};

