// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/M_Enemy_Area_Spawner.h"
#include	"Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/Enemy.h"



AM_Enemy_Area_Spawner::AM_Enemy_Area_Spawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;

	SpawnArea->SetBoxExtent(DimensionsOfBox);

}

void AM_Enemy_Area_Spawner::BeginPlay()
{
	Super::BeginPlay();
    if (HasAuthority())
    {
        SpawnEnemies();
    }
	
}

FVector AM_Enemy_Area_Spawner::GetRandomPointInBox() const
{
	FVector Center = SpawnArea->GetComponentLocation();
	FVector Extent = SpawnArea->GetScaledBoxExtent();

	return UKismetMathLibrary::RandomPointInBoundingBox(Center, Extent);
	
}

void AM_Enemy_Area_Spawner::SpawnEnemies()
{
    for (auto& Elem : EnemiesToSpawn)
    {
        TSubclassOf<AActor> EnemyClass = Elem.Key;
        int32 Amount = Elem.Value;

        if (!EnemyClass) continue;

        for (int32 i = 0; i < Amount; i++)
        {
            FVector SpawnLocation = GetRandomPointInBox();
            FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
        }
    }
}




