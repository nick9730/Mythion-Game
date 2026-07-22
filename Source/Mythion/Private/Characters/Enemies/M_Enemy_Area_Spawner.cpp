// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/M_Enemy_Area_Spawner.h"
#include	"Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/Enemy.h"



AM_Enemy_Area_Spawner::AM_Enemy_Area_Spawner()
{
    PrimaryActorTick.bCanEverTick = true;

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

void AM_Enemy_Area_Spawner::Tick(float DeltaTime)
{
    
        Super::Tick(DeltaTime);

      
    
}

FVector AM_Enemy_Area_Spawner::GetRandomPointInBox() const
{
	FVector Center = SpawnArea->GetComponentLocation();
	FVector Extent = SpawnArea->GetScaledBoxExtent();

	return UKismetMathLibrary::RandomPointInBoundingBox(Center, Extent);
	
}

bool AM_Enemy_Area_Spawner::IsLocationInsideBox(const FVector& PlayerLocation) const
{
    FVector BoxOrigin = SpawnArea->GetComponentLocation();
    FVector BoxExtent = SpawnArea->GetScaledBoxExtent();

   
    return FMath::Abs(PlayerLocation.X - BoxOrigin.X) <= BoxExtent.X &&
        FMath::Abs(PlayerLocation.Y - BoxOrigin.Y) <= BoxExtent.Y;
}

FVector AM_Enemy_Area_Spawner::GetSafeSpawnLocation(const FVector& PlayerLocation) const
{
    FVector BoxOrigin = SpawnArea->GetComponentLocation();
    FVector BoxExtent = SpawnArea->GetScaledBoxExtent();
    FVector Direction = (PlayerLocation - BoxOrigin).GetSafeNormal();
	FVector SafeLocation = BoxOrigin + Direction * (BoxExtent.Size() + PlayerCharacterSpawnRadius);
	return  SafeLocation;
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
			
         //   GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
            AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
            AEnemy* SpawnedEnemy = Cast<AEnemy>(SpawnedActor);
            if (IsValid(SpawnedEnemy))
                SpawnedEnemy->OwningSpawner = this;
        }
    }
}







