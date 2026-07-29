// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "CoreMinimal.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

#include "M_Enemy_Controller.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

class UAIPerceptionComponent;
class UBehaviorTree;
class UBlackboardData;
struct FTimerHandle;
class APawn;
class FName;
struct FAIStimulus;
class AActor;
template <typename T> struct FAINamedID;
struct FAISenseCounter;

UCLASS()
class MYTHION_API AM_Enemy_Controller : public AAIController
{
    GENERATED_BODY()

  public:
    AM_Enemy_Controller();

  protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn *InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent *EnemyPerceptionComponent;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor *> &UpdatedActors);

    UFUNCTION()
    void WhoIsTheClosestTarget(TArray<AActor *> Actors);

    UPROPERTY()
    AActor *ClosestTarget = nullptr;

    UFUNCTION()
    bool IsTargetDead(AActor *Actor);

    UFUNCTION()
    bool IsNotValidTarget(AActor *Actor);

    UPROPERTY()
    APawn *EnemyCharacter;

    UFUNCTION(BlueprintCallable)
    void SetEnemyToPassive();

    UFUNCTION()
    void SetEnemyToAggresive(AActor *Target);

    UFUNCTION()
    void SetEnemyToInvestigate();

    UFUNCTION()
    void SendEnemyToHomeLocation(AActor *Actor);

    UFUNCTION()
    void SetValueVector(FName BlackBoardKey, FVector TargetLocation);

    UFUNCTION()
    void HandlePerceptionMelee(const TArray<AActor *> &PerceivedActors, FAIStimulus Stimulus, AActor *Player,
                               AEnemy *Enemy, AM_PlayerController *PC);

    UFUNCTION()
    void HandlePerceptionMage(const TArray<AActor *> &PerceivedActors, FAIStimulus Stimulus, AActor *Player,
                              AEnemy *Enemy, AM_PlayerController *PC);

  private:
    UAISenseConfig_Sight *SightConfig;
    UAISenseConfig_Hearing *HearingConfig;
    UAISenseConfig_Damage *DamageConfig;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree *BehaviorTree;

    FName TargetActorKey = FName("TargetActor");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName AttackTargetKeyName = FName("TargetActor");

    UPROPERTY(EditAnywhere, Category = "Sight Settings")
    float SightRadius;

    UPROPERTY(EditAnywhere, Category = "Sight Settings")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, Category = "Sight Settings")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, Category = "Hear Settings")
    float HearingRange;

    FTimerHandle LoseSightTimer;

    // AI Movement
    UPROPERTY(EditAnywhere, Category = "AI")
    UBlackboardData *BlackboardAsset;
};
