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

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TMap<TSubclassOf<AEnemy>, int32> EnemiesToSpawn;


	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	UBoxComponent* SpawnArea;

	 
	void SpawnEnemies(); 

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector DimensionsOfBox= FVector(500.f, 500.f, 100.f);


};

