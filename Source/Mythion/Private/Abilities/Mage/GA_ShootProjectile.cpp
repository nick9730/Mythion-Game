// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/Mage/GA_ShootProjectile.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "GAS/M_GameplayAbility.h"
#include "Projectiles/M_BaseProjectile.h"
#include "Weapons/WeaponBase.h"

void UGA_ShootProjectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo *ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(ActorInfo->AvatarActor.Get());
    AEnemy *Enemy = Cast<AEnemy>(ActorInfo->AvatarActor.Get());

    if (IsValid(PlayerCharacter))
    {
        PlayerChar = PlayerCharacter;
        CurrentLevel = GetCharacterLevel(PlayerChar);
    }
    if (IsValid(Enemy))
    {
        EnemyChar = Enemy;
    }

    /*
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    */

    EventCalledTag = FGameplayTag::RequestGameplayTag(FName("Event.ShootProjectile"));
    WaitGameplayEvent(EventCalledTag);
    PlayMontage();
}


FVector UGA_ShootProjectile::PredictTargetLocation(AActor *TargetActor, FVector ShooterLocation, float ProjectileSpeed)
{
    if (!IsValid(TargetActor))
        return ShooterLocation;

    FVector TargetLocation = TargetActor->GetActorLocation();
    FVector TargetVelocity = TargetActor->GetVelocity();

    if (TargetVelocity.IsNearlyZero())
        return TargetLocation;

    FVector ToTarget = TargetLocation - ShooterLocation;
    float Distance = ToTarget.Size();

    float TimeToImpact = Distance / ProjectileSpeed;

    FVector PredictedLocation = TargetLocation + (TargetVelocity * TimeToImpact);

    return PredictedLocation;
}

void UGA_ShootProjectile::OnGameplayEventReceived(FGameplayEventData Payload)
{
    Super::OnGameplayEventReceived(Payload);

    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (IsValid(PlayerChar))
    {

        SpawnSingleProjectile(PlayerChar->GetActorForwardVector());
    }
    else if (IsValid(EnemyChar))
    {
        if (!IsValid(EnemyChar))
            return;

        AM_Enemy_Controller *EnemyController = Cast<AM_Enemy_Controller>(EnemyChar->GetController());
        if (!IsValid(EnemyController))
            return;

        UBlackboardComponent *BlackboardComp = EnemyController->GetBlackboardComponent();
        if (!IsValid(BlackboardComp))
            return;

        AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKeyName));

        FVector SpawnPoint = EnemyChar->GetMesh()->GetSocketLocation(EnemyChar->SocketName);
        FVector PredictedTargetPos = PredictTargetLocation(TargetActor, SpawnPoint, ProjectileSpeedForPrediction);
        FVector FinalDirectionOFProjectile = (PredictedTargetPos - SpawnPoint).GetSafeNormal();
        SpawnSingleProjectile(FinalDirectionOFProjectile);
    }
}

void UGA_ShootProjectile::HandleProjectileHit(AActor *HitActor, FVector HitLocation)
{
    FGameplayTag CueTag;
    float Magnitude = 1.f;
    if (IsValid(PlayerChar))
    {

        Magnitude = Damage * CurrentLevel;
    }
    else if (IsValid(EnemyChar))
    {
        Magnitude = Damage;
    }

    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Magical"));

    if (APlayerCharacter *PlayerCharacter = Cast<APlayerCharacter>(HitActor))
    {
        ApplyEffectWithMagnitude(PlayerCharacter, SetByCallerTag, Magnitude);
    }
    else if (AEnemy *EnemyTarget = Cast<AEnemy>(HitActor))
    {
        ApplyEffectWithMagnitude(EnemyTarget, SetByCallerTag, Magnitude);
    }
    ExecuteCueOnActor(HitActor, ProjectileCueTagEnd, Magnitude, HitLocation);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ShootProjectile::SpawnSingleProjectile(FVector Direction)
{
    FVector SpawnLocation;
    FActorSpawnParameters SpawnParams;

    if (!Projectile)
        return;

    if (IsValid(PlayerChar))
    {

        if (!IsValid(PlayerChar->EquippedWeapon))
            return;

        if (!IsValid(PlayerChar->EquippedWeapon->WeaponMesh))
            return;

        SpawnLocation =
            PlayerChar->EquippedWeapon->WeaponMesh->GetSocketLocation(FName("TraceStart")) + Direction * FrontDistance;
        SpawnParams.Owner = PlayerChar;
        SpawnParams.Instigator = PlayerChar;
    }
    else if (IsValid(EnemyChar))
    {
        SpawnLocation = EnemyChar->GetMesh()->GetSocketLocation(EnemyChar->SocketName);

        SpawnParams.Owner = EnemyChar;
        SpawnParams.Instigator = EnemyChar;
    }

    FRotator SpawnRotation = Direction.Rotation();

    AM_BaseProjectile *SpawnedProjectile =
        GetWorld()->SpawnActor<AM_BaseProjectile>(Projectile, SpawnLocation, SpawnRotation, SpawnParams);

    if (IsValid(SpawnedProjectile))
    {
        AActor *CueSourceActor =
            IsValid(PlayerChar) ? static_cast<AActor *>(PlayerChar) : static_cast<AActor *>(EnemyChar);
        ExecuteCueOnActor(PlayerChar, ProjectileCueTagStart, 400, SpawnLocation);
        if (IsValid(PlayerChar))
        {
            SpawnedProjectile->SetSourceActor(PlayerChar);
            SpawnedProjectile->IgnoreActor(PlayerChar);
        }

        else if (IsValid(EnemyChar))
        {
            SpawnedProjectile->SetSourceActor(EnemyChar);
            SpawnedProjectile->IgnoreActor(EnemyChar);
        }
        SpawnedProjectile->LaunchInDirection(Direction);
        SpawnedProjectile->OnProjectileHit.AddDynamic(this, &UGA_ShootProjectile::HandleProjectileHit);
        SpawnedProjectile->OnProjectileExpired.AddDynamic(this, &UGA_ShootProjectile::HandleProjectileExpired);
    }
}

void UGA_ShootProjectile::HandleProjectileExpired()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}