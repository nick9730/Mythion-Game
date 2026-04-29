// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "M_Enemy_Controller.generated.h"


class UAISenseConfig_Sight;
class UAIPerceptionComponent;
class UBehaviorTree;

UCLASS()
class MYTHION_API AM_Enemy_Controller : public AAIController
{
	GENERATED_BODY()
	

public:
	AM_Enemy_Controller();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;



	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* EnemyPerceptionComponent;


private:

	
	 UAISenseConfig_Sight* SightConfig;

	 UPROPERTY(EditDefaultsOnly, Category = "AI")
	 UBehaviorTree* BehaviorTree;

	 FName TargetActorKey = FName("TargetActor");

	 UPROPERTY(EditDefaultsOnly, Category = "AI")
	 FName AttackTargetKeyName = FName("TargetActor");
	 

	 UPROPERTY(EditAnywhere, Category = "Sight Settings")
	 float SightRadius = 1500.f;

	 UPROPERTY(EditAnywhere, Category = "Sight Settings")
	 float LoseSightRadius = 2000.f;


	 UPROPERTY(EditAnywhere, Category = "Sight Settings")
	 float PeripheralVisionAngleDegrees = 60.f;
};
