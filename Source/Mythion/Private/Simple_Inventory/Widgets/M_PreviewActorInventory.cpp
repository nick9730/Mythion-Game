// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/Widgets/M_PreviewActorInventory.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DataAsset/CharacterClasses.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Weapons/WeaponBase.h"
#include "Engine/TextureRenderTarget2D.h"



AM_PreviewActorInventory::AM_PreviewActorInventory()
{
    PrimaryActorTick.bCanEverTick = false;

    PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    RootComponent = PreviewMesh;

    CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
    CaptureComponent->SetupAttachment(RootComponent);
    CaptureComponent->SetRelativeLocation(FVector(-200.f, 0.f, 100.f));
    CaptureComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    CaptureComponent->bCaptureEveryFrame = true;
}

void AM_PreviewActorInventory::UpdateWeapon(FItemData WeaponItem)
{
    if (!IsValid(WeaponItem.ItemDataAsset)) return;

    // Spawn weapon και attach στο socket
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
        FString::Printf(TEXT("WeaponClass: %s"),
            IsValid(WeaponItem.ItemDataAsset->WeaponClass) ? *WeaponItem.ItemDataAsset->WeaponClass->GetName() : TEXT("NULL")));


    AWeaponBase* PreviewWeapon = GetWorld()->SpawnActor<AWeaponBase>(
        WeaponItem.ItemDataAsset->WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator);
    if (IsValid(PreviewWeapon))
        PreviewWeapon->AttachToComponent(PreviewMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HandRightSocket"));
}

void AM_PreviewActorInventory::UpdatePreviewMesh(FCharacterClassData ClassData)
{
    if (IsValid(PreviewMesh))
    {
        PreviewMesh->SetSkeletalMesh(ClassData.CharacterMesh);
        PreviewMesh->SetAnimInstanceClass(ClassData.AnimBlueprintClass);
    }
}

void AM_PreviewActorInventory::InitPreview(USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimClass)
{
    if (IsValid(Mesh))
        PreviewMesh->SetSkeletalMesh(Mesh);

    if (IsValid(AnimClass))
        PreviewMesh->SetAnimInstanceClass(AnimClass);

    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->InitAutoFormat(512, 512);
    RenderTarget->UpdateResourceImmediate(true);
    CaptureComponent->TextureTarget = RenderTarget;
}
