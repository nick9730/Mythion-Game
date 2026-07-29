// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Enemies/M_Enemy_Controller.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/Enemy_Controller_Types.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "M_PlayerController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

AM_Enemy_Controller::AM_Enemy_Controller()
{
    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerception"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

    if (SightConfig)
    {
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        EnemyPerceptionComponent->ConfigureSense(*SightConfig);
    }

    if (HearingConfig)
    {
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        EnemyPerceptionComponent->ConfigureSense(*HearingConfig);
    }
    if (DamageConfig)
    {
        EnemyPerceptionComponent->ConfigureSense(*DamageConfig);
    }
}

void AM_Enemy_Controller::BeginPlay()
{

    Super::BeginPlay();

    EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this,
                                                                   &AM_Enemy_Controller::OnTargetPerceptionUpdated);
    EnemyPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AM_Enemy_Controller::OnPerceptionUpdated);
}

void AM_Enemy_Controller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GetPawn())
    {
        SetControlRotation(GetPawn()->GetActorRotation());
    }
}

void AM_Enemy_Controller::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    AEnemy *Enemy = Cast<AEnemy>(GetPawn());

    if (!IsValid(Enemy))
        return;

    EnemyCharacter = Enemy;
    EnemyPerceptionComponent->ConfigureSense(*SightConfig);
    EnemyPerceptionComponent->ConfigureSense(*HearingConfig);
    EnemyPerceptionComponent->ConfigureSense(*DamageConfig);

    Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->WalkSpeed;
    if (InPawn && EnemyPerceptionComponent)
    {
        EnemyPerceptionComponent->RegisterComponent();
    }

    if (SightConfig && EnemyPerceptionComponent)
    {

        SightConfig->SightRadius = Enemy->SightRadius;
        SightConfig->LoseSightRadius = Enemy->LoseSightRadius;
        SightConfig->PeripheralVisionAngleDegrees = Enemy->PeripheralVisionAngleDegrees;

        EnemyPerceptionComponent->RequestStimuliListenerUpdate();
    }

    if (HearingConfig && EnemyPerceptionComponent)
    {
        HearingConfig->HearingRange = Enemy->HearingRange;
        EnemyPerceptionComponent->RequestStimuliListenerUpdate();
    }

    if (DamageConfig)
    {

        EnemyPerceptionComponent->RequestStimuliListenerUpdate();
    }
    if (BlackboardAsset)
    {
        UBlackboardComponent *BlackboardComp = nullptr;
        UseBlackboard(BlackboardAsset, BlackboardComp);
        if (BehaviorTree)
        {
            RunBehaviorTree(Enemy->BehaviorTree);
        }
    }

    SetValueVector(FName("HomeLocation"), InPawn->GetActorLocation());
}

void AM_Enemy_Controller::WhoIsTheClosestTarget(TArray<AActor *> Actors)
{
    EnemyPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), Actors);

    float MinDistance = MAX_FLT;
    ClosestTarget = nullptr;
    for (AActor *PerceivedActor : Actors)
    {
        if (!PerceivedActor->ActorHasTag(FName("Player")))
            continue;
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PerceivedActor->GetActorLocation());
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestTarget = PerceivedActor;
        }
    }
}

bool AM_Enemy_Controller::IsTargetDead(AActor *Actor)
{
    APlayerCharacter *PlayerTarget = Cast<APlayerCharacter>(Actor);
    if (IsValid(PlayerTarget))
    {
        UAbilitySystemComponent *TargetASC = PlayerTarget->GetAbilitySystemComponent();
        if (IsValid(TargetASC))
        {
            bool bIsDead = TargetASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Status.Dead")));
            if (bIsDead)
            {
                UE_LOG(LogTemp, Warning, TEXT("Nikosa"));
                GetBlackboardComponent()->ClearValue(FName("TargetActor"));
                GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Passive);
                return true;
            }
        }
    }
    return false;
}

bool AM_Enemy_Controller::IsNotValidTarget(AActor *Actor)
{
    APlayerCharacter *PlayerTarget = Cast<APlayerCharacter>(Actor);
    if (IsValid(PlayerTarget))
    {
        UAbilitySystemComponent *TargetASC = PlayerTarget->GetAbilitySystemComponent();
        if (!IsValid(TargetASC))
        {
            return true;
        }
    }
    return false;
}

void AM_Enemy_Controller::SetEnemyToPassive()
{

    GetBlackboardComponent()->ClearValue(FName("TargetActor"));
    GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Passive);

    /*
    if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
    {
        BTComp->RestartTree();
    }
    */
}

void AM_Enemy_Controller::SetEnemyToAggresive(AActor *Target)
{

    GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Target);
    GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Attacking);
}

void AM_Enemy_Controller::SetEnemyToInvestigate()
{
    GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Investigating);
}

void AM_Enemy_Controller::SendEnemyToHomeLocation(AActor *Actor)
{
    if (IsNotValidTarget(Actor))
    {
        GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Frozen);
    }
}

void AM_Enemy_Controller::SetValueVector(FName BlackBoardKey, FVector TargetLocation)
{
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsVector(BlackBoardKey, TargetLocation);
    }
}

void AM_Enemy_Controller::OnTargetPerceptionUpdated(AActor *Actor, FAIStimulus Stimulus)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Perception Updated Called"));
    if (!GetBlackboardComponent())
        return;
    AEnemy *Enemy = Cast<AEnemy>(GetPawn());
    if (!IsValid(Enemy) || !Enemy->IsAlive())
        return;

    APlayerCharacter *PlayerTarget = Cast<APlayerCharacter>(Actor);

    if (!IsValid(PlayerTarget))
        return;

    AM_PlayerController *PC = Cast<AM_PlayerController>(PlayerTarget->GetController());
    if (!IsValid(PC))
        return;

    TArray<AActor *> PerceivedActors;

    if (Enemy->EnemyType == EEnemyType::Mage)
    {
        // GetWorldTimerManager().ClearTimer(LoseSightTimer);
        HandlePerceptionMage(PerceivedActors, Stimulus, PlayerTarget, Enemy, PC);
    }
    else
    {
        HandlePerceptionMelee(PerceivedActors, Stimulus, PlayerTarget, Enemy, PC);
    }
}

void AM_Enemy_Controller::HandlePerceptionMelee(const TArray<AActor *> &PerceivedActors, FAIStimulus Stimulus,
                                                AActor *Player, AEnemy *Enemy, AM_PlayerController *PC)
{
    FAISenseID SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    FAISenseID HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
    FAISenseID DamageID = UAISense::GetSenseID(UAISense_Damage::StaticClass());
    if (!IsValid(Player))
        return;
    if (!IsValid(Enemy))
        return;

    if (!IsValid(PC))
        return;

    GetWorldTimerManager().ClearTimer(LoseSightTimer);
    if (Stimulus.Type == SightID)
    {

        // if (!Actor->ActorHasTag(FName("Player"))) return;

        if (Stimulus.WasSuccessfullySensed())
        {
            PC->Client_NotifyUserByEnemyPerception(true, Enemy);

            GetWorldTimerManager().ClearTimer(LoseSightTimer);

            if (IsTargetDead(Player))
            {
                SetEnemyToPassive();
                Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->WalkSpeed;
            }
            if (IsNotValidTarget(Player))
                SetEnemyToPassive();
            Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->ChaseSpeed;

            WhoIsTheClosestTarget(PerceivedActors);
            if (IsValid(ClosestTarget))
                SetEnemyToAggresive(ClosestTarget);
        }
        else
        {
            PC->Client_NotifyUserByEnemyPerception(false, Enemy);

            SetEnemyToInvestigate();
            SetValueVector(FName("LastKnownLocation"), Player->GetActorLocation());
            int8 CurrentState = GetBlackboardComponent()->GetValueAsEnum(FName("State"));
            if (CurrentState == (uint8)E_AIStates::Investigating)
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    PC->Client_NotifyUserByEnemyPerception(true, Enemy);

                    WhoIsTheClosestTarget(PerceivedActors);
                    if (IsValid(ClosestTarget))
                        SetEnemyToAggresive(ClosestTarget);
                }
                else
                {
                    PC->Client_NotifyUserByEnemyPerception(false, Enemy);
                    TArray<AActor *> TempActors = PerceivedActors;
                    TempActors.Remove(Player);
                    if (PerceivedActors.Num() == 0)
                    {
                        GetWorldTimerManager().SetTimer(LoseSightTimer, [this]() { SetEnemyToPassive(); }, 5.0f, false);
                    }
                }
            }
        }
    }
    else if (Stimulus.Type == DamageID)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            PC->Client_NotifyUserByEnemyPerception(true, Enemy);

            if (IsTargetDead(Player))
            {
                SetEnemyToPassive();
                Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->WalkSpeed;
            }
            if (IsNotValidTarget(Player))
                SetEnemyToPassive();

            Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->ChaseSpeed;

            GetWorldTimerManager().ClearTimer(LoseSightTimer);

            SetEnemyToAggresive(Player);
        }
        else
        {
            PC->Client_NotifyUserByEnemyPerception(false, Enemy);
        }
    }
    else if (Stimulus.Type == HearingID)
    {
        //  if (!Actor->ActorHasTag(FName("Player"))) return;

        if (Stimulus.WasSuccessfullySensed())
        {
            PC->Client_NotifyUserByEnemyPerception(true, Enemy);

            int8 CurrentState = GetBlackboardComponent()->GetValueAsEnum(FName("State"));
            if (CurrentState == (uint8)E_AIStates::Attacking)
            {
                return;
            }

            if (IsTargetDead(Player))
            {
                SetEnemyToPassive();
                Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->WalkSpeed;
            }
            if (IsNotValidTarget(Player))
                SetEnemyToPassive();

            Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->ChaseSpeed;

            GetWorldTimerManager().ClearTimer(LoseSightTimer);

            SetEnemyToInvestigate();
            SetValueVector(FName("LastKnownLocation"), Stimulus.StimulusLocation);

            GetWorldTimerManager().SetTimer(LoseSightTimer, [this]() { SetEnemyToPassive(); }, 5.0f, false);
        }
        else
        {
            PC->Client_NotifyUserByEnemyPerception(false, Enemy);
        }
    }
}
void AM_Enemy_Controller::HandlePerceptionMage(const TArray<AActor *> &PerceivedActors, FAIStimulus Stimulus,
                                               AActor *Player, AEnemy *Enemy, AM_PlayerController *PC)
{
    FAISenseID SightID = UAISense::GetSenseID(UAISense_Sight::StaticClass());
    FAISenseID HearingID = UAISense::GetSenseID(UAISense_Hearing::StaticClass());
    FAISenseID DamageID = UAISense::GetSenseID(UAISense_Damage::StaticClass());

    if (!IsValid(Player))
        return;
    if (!IsValid(Enemy))
        return;
    if (!IsValid(PC))
        return;

    GetWorldTimerManager().ClearTimer(LoseSightTimer);
    if (Stimulus.Type == SightID)
    {

        if (!Player->ActorHasTag(FName("Player")))
            return;

        if (Stimulus.WasSuccessfullySensed())
        {

            PC->Client_NotifyUserByEnemyPerception(true, Enemy);

            GetWorldTimerManager().ClearTimer(LoseSightTimer);
            if (IsNotValidTarget(Player))
                SetEnemyToPassive();
            Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->ChaseSpeed;

            WhoIsTheClosestTarget(PerceivedActors);
            if (IsValid(ClosestTarget))
                SetEnemyToAggresive(ClosestTarget);
        }
        else
        {
            PC->Client_NotifyUserByEnemyPerception(false, Enemy);
            TArray<AActor *> TempActors = PerceivedActors;
            TempActors.Remove(Player);
            if (PerceivedActors.Num() == 0)
            {
                GetWorldTimerManager().SetTimer(LoseSightTimer, [this]() { SetEnemyToPassive(); }, 5.0f, false);
            }
        }
    }
    else if (Stimulus.Type == DamageID)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            PC->Client_NotifyUserByEnemyPerception(true, Enemy);
            if (IsNotValidTarget(Player))
                SetEnemyToPassive();

            Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->ChaseSpeed;

            GetWorldTimerManager().ClearTimer(LoseSightTimer);

            SetEnemyToAggresive(Player);
        }
        else
        {
            PC->Client_NotifyUserByEnemyPerception(false, Enemy);
        }
    }
}

void AM_Enemy_Controller::OnPerceptionUpdated(const TArray<AActor *> &UpdatedActors)
{
    if (!GetBlackboardComponent())
        return;
    AEnemy *Enemy = Cast<AEnemy>(GetPawn());
    if (!IsValid(Enemy) || !Enemy->IsAlive())
        return;

    TArray<AActor *> PerceivedActors;

    EnemyPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    TArray<AActor *> Players;
    for (AActor *Actor : PerceivedActors)
    {
        if (!Actor->ActorHasTag(FName("Player")))
            continue;
        if (IsTargetDead(Actor))
            continue;

        Players.Add(Actor);
    }

    /*
    if (Players.Num() == 0)
    {
        GetWorldTimerManager().SetTimer(LoseSightTimer, [this]()
            {
                GetBlackboardComponent()->ClearValue(FName("TargetActor"));
                GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Passive);
            }, 3.f, false);
    }
    */
}
