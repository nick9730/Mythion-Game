// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Simple_Inventory/M_InventoryInterface.h"
#include "Simple_Inventory/Data/InventoryData.h"
#include "M_Interactive_Item.generated.h"


class UStaticMeshComponent;
class USphereComponent;
class UM_Item_Details;
class APlayerCharacter;



UCLASS()
class MYTHION_API AM_Interactive_Item : public AActor, public IM_InventoryInterface
{
	GENERATED_BODY()
	
public:
    AM_Interactive_Item();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

    UFUNCTION()
    void UpdateMesh();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    USphereComponent* InteractSphere;

    UPROPERTY(ReplicatedUsing = OnRep_ItemData, EditAnywhere, BlueprintReadOnly)
    UM_Item_Details* ItemData;

    UFUNCTION()
    void OnRep_ItemData();

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)

    int32 Quantity = 32; 
   





    //Inventory
    UFUNCTION(Server, Reliable)
    void Server_Interact(AActor* Caller);

    virtual void Interact(AActor* PlayerChar) override;
    virtual void OnFocused() override;
    virtual void OnUnfocused() override;
    static AM_Interactive_Item* SpawnItem(UWorld* World, UM_Item_Details* InItemData, int32 InQuantity, FVector Location, TSubclassOf<AM_Interactive_Item> ItemClass);

    UPROPERTY(EditAnywhere, Category = "Highlight")
    UMaterialInterface* HighlightMaterial;

    UPROPERTY()
    UMaterialInterface* OriginalMaterial;



    // Rotation speed
    UPROPERTY(EditAnywhere)
    float RotationSpeed = 90.f;


  




protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;




    private:
    FVector StartLocation;
    float TimeElapsed = 0.f;
   

    bool CantEquipWeapon(AActor* PlayerChar);
	bool CheckTheSpaceOfInventory(AActor* PlayerChar);
    bool HasRequiredClassTag(AActor* PlayerChar, FItemData Item);



};
