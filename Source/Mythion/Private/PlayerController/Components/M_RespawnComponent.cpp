// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_RespawnComponent.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "M_PlayerController.h"
#include "PlayerController/Components/M_BackendComponent.h"
#include "Respawn/M_SpawnPoints.h"
#include "Widgets/PlayerWidget/M_RespawnWidget.h"

// Sets default values for this component's properties
UM_RespawnComponent::UM_RespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

FVector UM_RespawnComponent::GetCorrectZLocation(FVector TargetLocation)
{
    FVector TraceStart = TargetLocation + FVector(0.f, 0.f, 1000.f);
    FVector TraceEnd = TargetLocation - FVector(0.f, 0.f, 1000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.bTraceComplex = false;

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (bHit)
    {
        return HitResult.Location + FVector(0.f, 0.f, 90.f);
    }

    return TargetLocation;
}

FVector UM_RespawnComponent::GetSafeRespawnPoint(FVector DeathLocation)
{
    TArray<AActor *> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AM_SpawnPoints::StaticClass(), SpawnPoints);

    TArray<AActor *> NearbyEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), NearbyEnemies);

    AM_SpawnPoints *BestSpawn = nullptr;
    float BestDistance = MAX_FLT;

    for (AActor *Actor : SpawnPoints)
    {
        AM_SpawnPoints *Spawn = Cast<AM_SpawnPoints>(Actor);
        if (!IsValid(Spawn))
            continue;

        bool bIsSafe = true;
        for (AActor *Enemy : NearbyEnemies)
        {
            if (!IsValid(Enemy))
                continue;
            if (FVector::Dist(Spawn->GetActorLocation(), Enemy->GetActorLocation()) < Spawn->SafeRadius)
            {
                bIsSafe = false;
                break;
            }
        }

        if (!bIsSafe)
            continue;

        float Distance = FVector::Dist(Spawn->GetActorLocation(), DeathLocation);
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            BestSpawn = Spawn;
        }
    }

    if (IsValid(BestSpawn))
        return BestSpawn->GetActorLocation();

    if (SpawnPoints.Num() > 0)
    {
        AActor *Fallback = SpawnPoints[0];
        if (IsValid(Fallback))
            return Fallback->GetActorLocation();
    }

    return DeathLocation;
}

void UM_RespawnComponent::Server_Respawn_Implementation()
{
    APlayerController *OwnerController = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *Char = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(Char))
        return;

    FVector TargetLocation = GetSafeRespawnPoint(Char->DeathLocation);

    Char->SetActorLocationAndRotation(TargetLocation, Char->GetActorRotation(), false, nullptr,
                                      ETeleportType::TeleportPhysics);

    Char->GetMesh()->SetSimulatePhysics(false);
    Char->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
    Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Char->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Char->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

    UAbilitySystemComponent *ASC = Char->GetAbilitySystemComponent();
    if (IsValid(ASC))
    {
        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Status.Dead"))));
        ASC->RemoveActiveEffects(Query);

        if (IsValid(Char->InitStatsEffect))
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Char->InitStatsEffect, 1, Context);
            float MaxHealth = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
            float MaxMana = ASC->GetNumericAttribute(UM_AttributeSet::GetMaxManaAttribute());
            float CurrentLevel = ASC->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
            float CurrentXP = ASC->GetNumericAttribute(UM_AttributeSet::GetXpAttribute());
            float CurrentXPMax = ASC->GetNumericAttribute(UM_AttributeSet::GetXpMaxAttribute());
            float CurrentCoins = ASC->GetNumericAttribute(UM_AttributeSet::GetCoinsAttribute());
            float CurrentArmor = ASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());
            float MagicResistance = ASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());
            float CurrentEnergy = 500.f;
            float CurrentMaxEnergy = 500.f;

            UM_AttributeSet::ApplyStatsFromBackend(ASC, Char->InitStatsEffect, MaxHealth, MaxMana, CurrentLevel,
                                                   CurrentXP, CurrentXPMax, CurrentCoins, CurrentArmor, MagicResistance,
                                                   CurrentEnergy, CurrentMaxEnergy, MaxHealth, MaxMana);
        }
    }

    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);
    if (IsValid(PCController) && IsValid(PCController->BackendComponent))
    {
        PCController->BackendComponent->Client_RequestStatsSave();
    }

    Client_HideRespawnWidget();
}

void UM_RespawnComponent::Client_ShowRespawnWidget_Implementation()
{
    if (!RespawnWidgetClass)
        return;

    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    FTimerHandle DelayTimer;
    TWeakObjectPtr<UM_RespawnComponent> WeakThis(this);
    TWeakObjectPtr<APlayerController> WeakOwner(OwnerPC);

    GetWorld()->GetTimerManager().SetTimer(
        DelayTimer,
        [WeakThis, WeakOwner]() {
            if (!WeakThis.IsValid() || !WeakOwner.IsValid())
                return;

            if (!IsValid(WeakThis->RespawnWidget))
            {
                WeakThis->RespawnWidget = CreateWidget<UM_RespawnWidget>(WeakOwner.Get(), WeakThis->RespawnWidgetClass);
                if (!IsValid(WeakThis->RespawnWidget))
                    return;
                WeakThis->RespawnWidget->AddToViewport();
            }
            WeakThis->RespawnWidget->SetVisibility(ESlateVisibility::Visible);

            WeakOwner->SetInputMode(FInputModeUIOnly());
            WeakOwner->bShowMouseCursor = true;
        },
        3.f, false);
}

void UM_RespawnComponent::Client_HideRespawnWidget_Implementation()
{
    if (IsValid(RespawnWidget))
        RespawnWidget->SetVisibility(ESlateVisibility::Hidden);

    APlayerController *PC = Cast<APlayerController>(GetOwner());
    if (IsValid(PC))
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}
