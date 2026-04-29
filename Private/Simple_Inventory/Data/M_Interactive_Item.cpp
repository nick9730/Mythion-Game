// Fill out your copyright notice in the Description page of Project Settings.


#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Characters/PlayerCharacter.h"
#include "Simple_Inventory/InventoryComponent.h"

#include "M_PlayerController.h"




// Sets default values
AM_Interactive_Item::AM_Interactive_Item()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(RootComponent);
}



void AM_Interactive_Item::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();

	if (IsValid(ItemData) && IsValid(ItemData->StaticMesh))
	{
		Mesh->SetStaticMesh(ItemData->StaticMesh);
	}
	if (IsValid(Mesh))
		OriginalMaterial = Mesh->GetMaterial(0);

}

void AM_Interactive_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));

	TimeElapsed += DeltaTime;
	float NewZ = StartLocation.Z + FMath::Sin(TimeElapsed) * 20.f;
	SetActorLocation(FVector(StartLocation.X, StartLocation.Y, NewZ));
}



void AM_Interactive_Item::Server_Interact_Implementation(AActor* Caller)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Server_Interact called!"));
	Interact(Caller);
}

void AM_Interactive_Item::Interact(AActor* PlayerChar)
{

	APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerChar);
	UInventoryComponent* InvComp = Player->InventoryComponent;

	if (!HasAuthority()) return;
	if (!IsValid(Player) || !IsValid(ItemData)) return;
	if (!IsValid(InvComp)) return;



	bool bHasSpace = false;
	for (const FItemData& Slot : InvComp->Items)
	{
		if (!Slot.IsValid()) { bHasSpace = true; break; }
		if (Slot.ItemID == ItemData->ItemID && Slot.Quantity < Slot.MaxStackSize) { bHasSpace = true; break; }
	}



	FItemData NewItem;
	NewItem.ItemID = ItemData->ItemID;
	NewItem.ItemName = ItemData->ItemName;
	NewItem.Icon = ItemData->Icon;
	NewItem.ItemType = ItemData->ItemType;
	NewItem.MaxStackSize = ItemData->MaxStackSize;
	NewItem.Quantity = Quantity;
	NewItem.ItemDataAsset = ItemData;
	
	InvComp->Server_AddItem(NewItem);
	if (!bHasSpace) return;

		Destroy();
}

void AM_Interactive_Item::OnFocused()
{

	if (IsValid(Mesh) && IsValid(HighlightMaterial))
		Mesh->SetMaterial(0, HighlightMaterial);

}

void AM_Interactive_Item::OnUnfocused()
{
	
	if (IsValid(Mesh) && IsValid(OriginalMaterial))
		Mesh->SetMaterial(0, OriginalMaterial);
}

AM_Interactive_Item* AM_Interactive_Item::SpawnItem(UWorld* World, UM_Item_Details* InItemData, int32 InQuantity, FVector Location, TSubclassOf<AM_Interactive_Item> ItemClass)
{
	if (!IsValid(World) || !IsValid(InItemData) || !IsValid(ItemClass)) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AM_Interactive_Item* SpawnedItem = World->SpawnActor<AM_Interactive_Item>(
		ItemClass, Location, FRotator::ZeroRotator, SpawnParams
	);

	if (IsValid(SpawnedItem))
	{
		SpawnedItem->ItemData = InItemData;
		SpawnedItem->Quantity = InQuantity;
		SpawnedItem->StartLocation = Location;
	}

	return SpawnedItem;
}




