// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Attributes/M_AttributeSet.h"
#include  "GameplayTagContainer.h"
#include "Characters/BaseCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"




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

void AWeaponBase::Server_ApplyDamage_Implementation(AActor* HitActor)
{
    ApplyDamage(HitActor);
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
    if (WeaponMesh && WeaponMeshToUse) {

        UStaticMesh* PreviousMesh = WeaponMesh->GetStaticMesh();
        WeaponMesh->SetStaticMesh(WeaponMeshToUse);
        WeaponMesh->MarkRenderStateDirty();
    }
}

void AWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority() && bIsScanning)
        PerformWeaponTrace();
}

bool AWeaponBase::SetHitScanEnabled(bool bEnabled)
{
    bIsScanning = bEnabled;
    if (bEnabled) AlreadyHitActors.Empty();
    return bIsScanning;
}

void AWeaponBase::PerformWeaponTrace()
{
    FVector Start = WeaponMesh->GetSocketLocation(SocketStart);
    FVector End = WeaponMesh->GetSocketLocation(SocketEnd);

    FVector Direction = (End - Start).GetSafeNormal();
    FVector EffectiveStart = Start + (Direction * 20.f);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore = { this, GetOwner() };

    bool bHit = UKismetSystemLibrary::SphereTraceMulti(
        this,
        EffectiveStart,
        End,
        12.f,
        UEngineTypes::ConvertToTraceType(ECC_Pawn),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::ForDuration,
        HitResults,
        true
    );

    if (!bHit) return;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == this || HitActor == GetOwner()) continue;
        if (AlreadyHitActors.Contains(HitActor)) continue;

        AlreadyHitActors.Add(HitActor);
        if (HasAuthority())
        {
            ApplyDamage(HitActor);
        }
        else
        {
            Server_ApplyDamage(HitActor);
        }
    }
}

void AWeaponBase::ApplyDamage(AActor* HitActor)
{

    if (!HasAuthority()) return;

    ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(HitActor);
    if (!IsValid(TargetCharacter)) return;

    UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();
    if (!IsValid(TargetASC)) return;


    if (!TargetASC->AbilityActorInfo.IsValid())
    {
        return;
    }

    ABaseCharacter* SourceCharacter = Cast<ABaseCharacter>(GetOwner());
    if (!IsValid(SourceCharacter)) return;

    UAbilitySystemComponent* SourceASC = SourceCharacter->GetAbilitySystemComponent();
    if (!IsValid(SourceASC)) return;

    if (!SourceASC->AbilityActorInfo.IsValid())
    {
        return;
    }



    float FinalDamage = WeaponDamage;
    float Armor = TargetASC->GetNumericAttribute(UM_AttributeSet::GetArmorAttribute());
    float MagicResistance = TargetASC->GetNumericAttribute(UM_AttributeSet::GetMagicResistanceAttribute());

    if (DamageTypeTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Damage.Type.Physical"))))
    {
        FinalDamage *= (100.f / (100.f + Armor));
    }
    else if (DamageTypeTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Damage.Type.Magical"))))
    {
        FinalDamage *= (100.f / (100.f + MagicResistance));
    }



    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    Context.AddInstigator(SourceCharacter, this);
    FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);

    if (!Spec.IsValid()) return;

    FGameplayTag DataDamageTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage"));
    Spec.Data->SetSetByCallerMagnitude(DataDamageTag, -FinalDamage);

    FActiveGameplayEffectHandle ActiveHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);

 
}