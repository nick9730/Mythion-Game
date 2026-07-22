#include "Weapons/WeaponBase.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Attributes/M_AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/Enemy.h"
#include "Perception/AISense_Damage.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    WeaponMesh->SetCanEverAffectNavigation(false);
    WeaponMesh->SetIsReplicated(true);
}

void AWeaponBase::Server_SendGameplayEvent_Implementation(AActor* InTarget, float InDamage, FGameplayTag InDamageTag, FHitResult InHitResult)
{
  
    if (!InTarget) return;


    AActor* Attacker = GetOwner();
    if (!Attacker) return;

    if (Attacker->IsA<AEnemy>() && InTarget->IsA<AEnemy>())
    {
        return;
    }

    if (AlreadyHitActorsServer.Contains(InTarget)) return;
    AlreadyHitActorsServer.Add(InTarget);

    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
    if (IsValid(SourceASC))
    {
        FGameplayEventData EventData;
        EventData.Instigator = GetOwner();
        EventData.Target = InTarget;

        FGameplayAbilityTargetDataHandle TargetDataHandle;
        FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(InHitResult);
        TargetDataHandle.Add(NewData);
        EventData.TargetData = TargetDataHandle;


        EventData.EventMagnitude = InDamage;
        EventData.InstigatorTags.AddTag(InDamageTag);

        FGameplayTag HitTag = FGameplayTag::RequestGameplayTag(FName("Event.Combat.MeleeHit"));

        FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
        ContextHandle.AddHitResult(InHitResult);
        EventData.ContextHandle = ContextHandle;

        SourceASC->HandleGameplayEvent(HitTag, &EventData);
    }

/*
    UAISense_Damage::ReportDamageEvent(
        GetWorld(),
        InTarget,
        Attacker,
        InDamage,
        InHitResult.Location,
        InHitResult.ImpactPoint
    );
*/
}

bool AWeaponBase::Server_SendGameplayEvent_Validate(AActor* InTarget, float InDamage, FGameplayTag InDamageTag, FHitResult InHitResult)
{
    return true;
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWeaponBase, WeaponMeshToUse);
}

void AWeaponBase::SetWeaponMesh(UStaticMesh* NewMesh)
{
    if (!HasAuthority()) return;
    WeaponMeshToUse = NewMesh;
    OnRep_WeaponMesh();
}

void AWeaponBase::OnRep_WeaponMesh()
{
    if (WeaponMesh && WeaponMeshToUse)
    {
        WeaponMesh->SetStaticMesh(nullptr);
        WeaponMesh->SetStaticMesh(WeaponMeshToUse);
        WeaponMesh->MarkRenderStateDirty();
    }
}

void AWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* OwningPawn = Cast<APawn>(GetOwner());
    if (bIsScanning )
    {
        PerformWeaponTrace();
    }
}

bool AWeaponBase::SetHitScanEnabled(bool bEnabled)
{
    bIsScanning = bEnabled;
    if (bEnabled)
    {
    AlreadyHitActors.Empty();
    AlreadyHitActorsServer.Empty();
    }
    return bIsScanning;
}

void AWeaponBase::PerformWeaponTrace()
{
    FVector Start = WeaponMesh->GetSocketLocation(SocketStart);
    FVector End = WeaponMesh->GetSocketLocation(SocketEnd);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore = { this, GetOwner() };

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        this, Start, End, 70.f,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true
    );

    if (!bHit) return;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == this || HitActor == GetOwner()) continue;
        if (AlreadyHitActors.Contains(HitActor)) continue;

        AlreadyHitActors.Add(HitActor);

        Server_SendGameplayEvent(HitActor, WeaponDamage, DamageTypeTag, Hit);

     
    }
}