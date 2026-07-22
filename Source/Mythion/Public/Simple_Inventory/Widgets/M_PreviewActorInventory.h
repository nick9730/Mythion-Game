// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataAsset/CharacterClasses.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "M_PreviewActorInventory.generated.h"




UCLASS()
class MYTHION_API AM_PreviewActorInventory : public AActor
{
	GENERATED_BODY()
	
public:
    AM_PreviewActorInventory();

    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* PreviewMesh;

    UPROPERTY(VisibleAnywhere)
    USceneCaptureComponent2D* CaptureComponent;

    
    UFUNCTION()
    void UpdateWeapon(FItemData WeaponItem);

    UFUNCTION()
    void UpdatePreviewMesh(FCharacterClassData ClassData);

    UPROPERTY()
    UTextureRenderTarget2D* RenderTarget;

    void InitPreview(USkeletalMesh* Mesh, TSubclassOf<UAnimInstance> AnimClass);
    UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

};
