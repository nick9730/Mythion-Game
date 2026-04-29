// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "CharactersEnemies.generated.h"

/**
 * 
 */

class USkeletalMesh;
class AnimationAsset;




USTRUCT(BlueprintType)
struct FEnemyStruct
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere,  Category = "Stats")
	float MaxHealth;
	UPROPERTY(EditAnywhere ,Category = "Stats")
	float Health;
	UPROPERTY(EditAnywhere, Category = "Stats")
	float Mana;
	UPROPERTY(EditAnywhere, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> EnemyMesh;
	UPROPERTY(EditAnywhere, Category = "Visuals")
	TSubclassOf<UAnimInstance> EnemyAnimation;

	UPROPERTY(EditAnywhere, Category = "Visuals")
	FVector MeshOffsetLocation = FVector(0.f, 0.f, -90.f);

	UPROPERTY(EditAnywhere, Category = "Visuals")
	FRotator MeshOffsetRotation = FRotator(0.f, -90.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Mythion|Abilities")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffect;
};


UCLASS()
class MYTHION_API UCharactersEnemies : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies Configuration")
	TArray<FEnemyStruct> EnemiesStats;

	
};
