// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M_Enemy_Area_Spawner.generated.h"


class UBoxComponent;
class AEnemy;


UCLASS()
class MYTHION_API AM_Enemy_Area_Spawner : public AActor
{
	GENERATED_BODY()
	
public:	

	AM_Enemy_Area_Spawner();

	FVector GetRandomPointInBox() const;

	bool IsLocationInsideBox(const FVector& PlayerLocation) const;
	FVector GetSafeSpawnLocation(const FVector& PlayerLocation) const;

	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	UBoxComponent* SpawnArea;


protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TMap<TSubclassOf<AEnemy>, int32> EnemiesToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawning PlayerCharacter")
	float PlayerCharacterSpawnRadius = 200.f;

	 
	void SpawnEnemies(); 

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector DimensionsOfBox= FVector(500.f, 500.f, 100.f);


};

