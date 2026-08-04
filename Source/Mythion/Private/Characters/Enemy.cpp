// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/Enemy.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Characters/Enemies/Enemy_Controller_Types.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemies/M_Enemy_Verticals.h"
#include "Characters/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/M_AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "M_PlayerController.h"
#include "M_QuestComponent.h"
#include "Weapons/WeaponBase.h"

AEnemy::AEnemy()
{
    AbilitySystemComponent = CreateDefaultSubobject<UM_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    AttributeSet = CreateDefaultSubobject<UM_AttributeSet>(TEXT("AttributeSet"));

    HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
    HealthBarComponent->SetupAttachment(RootComponent);
    HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarComponent->SetDrawSize(FVector2D(200.f, 40.f));
    HealthBarComponent->SetVisibility(false);
    bReplicates = true;
    SetReplicateMovement(true);
}

UAbilitySystemComponent *AEnemy::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AEnemy::Tick(float DeltaTime)
{
    {
        Super::Tick(DeltaTime);

        if (!HealthBarComponent)
            return;

        APlayerController *PC = GetWorld()->GetFirstPlayerController();
        if (!PC)
            return;

        APawn *PlayerPawn = PC->GetPawn();
        if (!PlayerPawn)
            return;

        float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
        bool bShouldShow = Distance <= HealthBarVisibilityRadius;

        HealthBarComponent->SetVisibility(bShouldShow);
    }
}

void AEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AEnemy::Multicast_PlayDeathAnimation_Implementation()
{
    if (IsValid(DeathMontage))
    {
        float Duration = PlayAnimMontage(DeathMontage);

        FTimerHandle DeathTimerHandle;
        GetWorldTimerManager().SetTimer(
            DeathTimerHandle,
            [this]() {
                if (IsValid(GetMesh()))
                {
                    GetMesh()->bPauseAnims = true;
                }
            },
            Duration, false);
    }
}

void AEnemy::FindTheCorrectEnemy()
{

    if (!HasAuthority())
        return;

    UE_LOG(LogTemp, Warning, TEXT("[QuestKill] FindTheCorrectEnemy called. LastDamageInstigator valid: %s"),
           IsValid(LastDamageInstigator) ? TEXT("YES") : TEXT("NO"));

    if (!IsValid(LastDamageInstigator))
        return;

    APlayerCharacter *KillerChar = Cast<APlayerCharacter>(LastDamageInstigator->GetPawn());
    UE_LOG(LogTemp, Warning, TEXT("[QuestKill] KillerChar valid: %s"), IsValid(KillerChar) ? TEXT("YES") : TEXT("NO"));

    if (!IsValid(KillerChar))
        return;

    UM_QuestComponent *QuestComp = KillerChar->FindComponentByClass<UM_QuestComponent>();
    UE_LOG(LogTemp, Warning, TEXT("[QuestKill] QuestComp valid: %s"), IsValid(QuestComp) ? TEXT("YES") : TEXT("NO"));

    if (!IsValid(QuestComp))
        return;

    QuestComp->OnEnemyKilled(GetClass(), LastDamageInstigator);
    UE_LOG(LogTemp, Warning, TEXT("[QuestKill] OnEnemyKilled called for class %s"), *GetClass()->GetName());
}

void AEnemy::BeginPlay()
{
    Super::BeginPlay();

    if (IsValid(AbilitySystemComponent))
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);

        if (HasAuthority())
        {
            if (IsValid(InitializeAttributesEffectEnemy))
            {
                InitializeEnemyAttributes(InitializeAttributesEffectEnemy);
            }

            for (TSubclassOf<UGameplayAbility> Ability : AbilityClasses)
            {
                if (IsValid(Ability))
                {
                    AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1));
                }
            }
            AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetHealthAttribute())
                .AddUObject(this, &ABaseCharacter::OnHealthChanged);
        }
    }

    if (HealthBarWidgetClass && HealthBarComponent)
    {
        HealthBarComponent->SetWidgetClass(HealthBarWidgetClass);
        HealthBarComponent->InitWidget();
        HealthBarComponent->SetVisibility(true);

        if (UM_Enemy_Verticals *Widget = Cast<UM_Enemy_Verticals>(HealthBarComponent->GetWidget()))
        {
            Widget->SetOwningEnemy(this);

            if (IsValid(AbilitySystemComponent))
            {
                int32 Level = FMath::FloorToInt(
                    AbilitySystemComponent->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute()));
                Widget->InitializeWidget(AbilitySystemComponent, EnemyDisplayName, Level);
            }
        }
    }

    if (HasAuthority())
    {
        if (WeaponData)
        {
            SpawnWeapon();
        }
    }

    SpawnPoint = GetActorLocation();
}

void AEnemy::PossessedBy(AController *NewController)
{
    Super::PossessedBy(NewController);
}

void AEnemy::HandleDeath()
{

    Super::HandleDeath();

    UE_LOG(LogTemp, Warning, TEXT("[QuestKill] HandleDeath called!"));
    if (!bAlive)
        return;
    bAlive = false;
    AM_Enemy_Controller *AIController = Cast<AM_Enemy_Controller>(GetController());
    if (IsValid(AIController) && IsValid(AIController->GetBlackboardComponent()))
    {
        AIController->GetBlackboardComponent()->SetValueAsEnum(FName("State"), (uint8)E_AIStates::Dead);
        AIController->StopMovement();
        AIController->BrainComponent->StopLogic(TEXT("Dead"));
    }

    if (HasAuthority())
    {
        Multicast_HideWeapon();
        Multicast_PlayDeathAnimation();
        Multicast_DisableCollisionOnDeath();

        if (IsValid(EquippedWeapon))
        {
            EquippedWeapon->SetLifeSpan(0.1f);
            EquippedWeapon = nullptr;
        }
    }

    GetCharacterMovement()->DisableMovement();
    SetLifeSpan(3.f);
    if (HasAuthority())
    {
        SpawnXpReward();
        SpawnActor();
    }
    FindTheCorrectEnemy();
}

void AEnemy::SpawnXpReward()
{
    if (!HasAuthority())
        return;
    if (!IsValid(XpRewardClass))
        return;

    FVector SpawnLocation = GetActorLocation();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AActor>(XpRewardClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

void AEnemy::SpawnActor()
{

    if (!HasAuthority())
        return;
    FVector CachedSpawnPoint = SpawnPoint;
    FRotator CachedRotation = GetActorRotation();
    TSubclassOf<AEnemy> EnemyClass = GetClass();
    AM_Enemy_Area_Spawner *CachedOwningSpawner = OwningSpawner;
    UWorld *World = GetWorld();

    FTimerHandle LocalTimer;
    World->GetTimerManager().SetTimer(
        LocalTimer,
        [World, CachedSpawnPoint, CachedRotation, EnemyClass, CachedOwningSpawner]() {
            if (!IsValid(World))
                return;
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            AEnemy *NewEnemy = World->SpawnActor<AEnemy>(EnemyClass, CachedSpawnPoint, CachedRotation, Params);
            if (IsValid(NewEnemy))
                NewEnemy->OwningSpawner = CachedOwningSpawner;
        },
        30.f, false);
}

void AEnemy::InitializeEnemyAttributes(TSubclassOf<UGameplayEffect> InitEffect)
{
    if (!IsValid(InitEffect) || !IsValid(AbilitySystemComponent))
        return;
    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1.f, Context);
    AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void AEnemy::Multicast_HideWeapon_Implementation()
{
    if (IsValid(EquippedWeapon))
        EquippedWeapon->SetActorHiddenInGame(true);
}

void AEnemy::Multicast_DisableCollisionOnDeath_Implementation()
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}