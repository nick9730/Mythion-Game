// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/M_MiniMap.h"

#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Characters/Enemy.h"
#include "Shop/M_MerchantNPC.h"
#include "AM_QuestNPC.h"

#include "Kismet/GameplayStatics.h"

void UM_MiniMap::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Img_Background) && IsValid(BackgroundTexture))
	{
		Img_Background->SetBrushFromTexture(BackgroundTexture);
	}

	if (UCanvasPanelSlot* PlayerSlot = Cast<UCanvasPanelSlot>(Img_PlayerIcon->Slot))
	{
		PlayerSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		PlayerSlot->SetAlignment(FVector2D(0.5f, 0.5f)); 
		PlayerSlot->SetPosition(FVector2D(WindowSize * 0.5f, WindowSize * 0.5f)); 

	}

	if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Img_Background->Slot))
	{
		BgSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		BgSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	}

	GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &UM_MiniMap::UpdateMiniMap, 0.1f, true);
}

void UM_MiniMap::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
	Super::NativeDestruct();
}

FVector2D UM_MiniMap::WorldToScaledPixel(FVector WorldLocation) const
{

	return FVector2D::ZeroVector;
}

void UM_MiniMap::UpdateMiniMap()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!IsValid(PlayerPawn)) return;
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float Scale = (WindowSize * 0.5f) / WorldRadius; 

	const float MinX = -20495.23f, MaxX = 15355.93f;
	const float MinY = -12130.20f, MaxY = 11334.46f;

	float BgWidth = (MaxX - MinX) * Scale;
	float BgHeight = (MaxY - MinY) * Scale;
	float BgSize = FMath::Max(BgWidth, BgHeight); 
	BgWidth = BgSize;
	BgHeight = BgSize;

	float CenterWorldX = (MinX + MaxX) * 0.5f;
	float CenterWorldY = (MinY + MaxY) * 0.5f;

	FVector2D BgOffsetFromPlayer = FVector2D(
		-(CenterWorldX - PlayerLocation.X) * Scale,
		-(CenterWorldY - PlayerLocation.Y) * Scale
	);

	if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(Img_Background->Slot))
	{
		BgSlot->SetSize(FVector2D(BgWidth, BgHeight));
		BgSlot->SetPosition(FVector2D(WindowSize * 0.5f, WindowSize * 0.5f) + BgOffsetFromPlayer);
	}


	if (IsValid(Img_PlayerIcon))
	{
		float PlayerYaw = PlayerPawn->GetActorRotation().Yaw;
		Img_PlayerIcon->SetRenderTransformAngle(PlayerYaw + RotationOffset);
	}

	UpdateIconsForClass(AEnemy::StaticClass(), EnemyIconTexture, EnemyIconWidgets, PlayerLocation);
	UpdateIconsForClass(QuestNPCClass, QuestIconTexture, QuestIconWidgets, PlayerLocation);
	UpdateIconsForClass(MerchantNPCClass, MerchantIconTexture, MerchantIconWidgets, PlayerLocation);
}

void UM_MiniMap::UpdateIconsForClass(TSubclassOf<AActor> ActorClass, UTexture2D* IconTexture,
	TMap<AActor*, UImage*>& IconMap, FVector PlayerLocation)
{
	if (!IsValid(ActorClass) || !IsValid(IconTexture) || !IsValid(RotatingCanvas)) return;

	float Scale = (WindowSize * 0.5f) / WorldRadius;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);

	TSet<AActor*> StillActive;

	for (AActor* Actor : FoundActors)
	{
		if (!IsValid(Actor)) continue;

		float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
		if (Distance > DetectionRadius) continue;

		StillActive.Add(Actor);

		FVector2D Delta = FVector2D(
			-(Actor->GetActorLocation().X - PlayerLocation.X) * Scale,
			-(Actor->GetActorLocation().Y - PlayerLocation.Y) * Scale
		);
		FVector2D IconPos = FVector2D(WindowSize * 0.5f, WindowSize * 0.5f) + Delta;

		UImage** ExistingIconPtr = IconMap.Find(Actor);
		UImage* IconWidget = (ExistingIconPtr && IsValid(*ExistingIconPtr)) ? *ExistingIconPtr : nullptr;

		if (!IconWidget)
		{
			IconWidget = NewObject<UImage>(this);
			if (!IsValid(IconWidget)) continue;

			IconWidget->SetBrushFromTexture(IconTexture);
			RotatingCanvas->AddChildToCanvas(IconWidget);

			if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
			{
				NewSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
				NewSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				NewSlot->SetAutoSize(false);
				NewSlot->SetSize(FVector2D(10.f, 10.f));
			}

			IconMap.Add(Actor, IconWidget);
		}

		if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
		{
			IconSlot->SetPosition(IconPos);
		}
	}

	TArray<AActor*> ToRemove;
	for (auto& Pair : IconMap)
		if (!StillActive.Contains(Pair.Key))
		{
			if (IsValid(Pair.Value)) Pair.Value->RemoveFromParent();
			ToRemove.Add(Pair.Key);
			for (AActor* Key : ToRemove)
			{
				IconMap.Remove(Key);
			}
		}
	}

