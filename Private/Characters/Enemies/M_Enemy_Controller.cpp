// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/M_Enemy_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Characters/Enemy.h"
#include "Characters/Enemies/Enemy_Controller_Types.h"


AM_Enemy_Controller::AM_Enemy_Controller()
{
    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	if (SightConfig) {
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
        
        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
	}

}

void AM_Enemy_Controller::BeginPlay()
{
	Super::BeginPlay();
    GetBlackboardComponent()->SetValueAsEnum(FName("States"), (uint8)E_AIStates::Attacking);
    UE_LOG(LogTemp, Warning, TEXT("EnumKey value after set: %d"), GetBlackboardComponent()->GetValueAsEnum(FName("State")));


}

void AM_Enemy_Controller::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);


}


