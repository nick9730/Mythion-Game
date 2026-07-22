// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ANS_MeleeHitScan.h"
#include "Weapons/WeaponBase.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"

void UANS_MeleeHitScan::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner)) return;


    ABaseCharacter* Character = Cast<ABaseCharacter>(Owner);
    if (!IsValid(Character)) return;

    AWeaponBase* Weapon = Cast<AWeaponBase>(Character->EquippedWeapon);
    if (!IsValid(Weapon)) return;

    Weapon->SetHitScanEnabled(true);
}

void UANS_MeleeHitScan::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner)) return;

    ABaseCharacter* Character = Cast<ABaseCharacter>(Owner);
    if (!IsValid(Character)) return;

    AWeaponBase* Weapon = Cast<AWeaponBase>(Character->EquippedWeapon);
    if (!IsValid(Weapon)) return;

    Weapon->SetHitScanEnabled(false);
}