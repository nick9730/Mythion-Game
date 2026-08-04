// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectiles/M_BaseProjectile.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GAS/M_GameplayAbility.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

// Sets default values
AM_BaseProjectile::AM_BaseProjectile()
{

    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->OnComponentHit.AddDynamic(this, &AM_BaseProjectile::OnHit);
    RootComponent = CollisionComponent;

    ProjectileVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVFX"));
    ProjectileVFX->SetupAttachment(RootComponent);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
    ProjectileMovement->ProjectileGravityScale = GravityScale;

    InitialLifeSpan = MaxLifetime;
}

void AM_BaseProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AM_BaseProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AM_BaseProjectile::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp,
                              FVector NormalImpulse, const FHitResult &Hit)
{
    if (!IsValid(OtherActor) || OtherActor == this || OtherActor == SourceActor)
        return;

    APawn *HitPawn = Cast<APawn>(OtherActor);
    if (!IsValid(HitPawn))
        return;

    OnProjectileHit.Broadcast(OtherActor, Hit.Location);

    Destroy();
}

void AM_BaseProjectile::Destroyed()
{
    if (!bHasHit)
    {
        OnProjectileExpired.Broadcast();
    }

    Super::Destroyed();
}

void AM_BaseProjectile::LaunchInDirection(FVector Direction)
{
    if (!IsValid(ProjectileMovement))
        return;
    ProjectileMovement->Velocity = Direction.GetSafeNormal() * ProjectileMovement->InitialSpeed;
}

void AM_BaseProjectile::IgnoreActor(AActor *ActorToIgnore)
{
    if (IsValid(CollisionComponent) && IsValid(ActorToIgnore))
    {
        CollisionComponent->IgnoreActorWhenMoving(ActorToIgnore, true);
    }
}