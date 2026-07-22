// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/M_Map.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/Enemy.h"
#include "Kismet/GameplayStatics.h"


void UM_Map::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Img_Background) && IsValid(FullMapBackgroundTexture))
	{
		Img_Background->SetBrushFromTexture(FullMapBackgroundTexture);
	}

	if (IsValid(Img_PlayerIcon))
	{
		if (UCanvasPanelSlot* PlayerSlot = Cast<UCanvasPanelSlot>(Img_PlayerIcon->Slot))
		{
			PlayerSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
			PlayerSlot->SetAlignment(FVector2D(0.5f, 0.5f)); 
		}
	}

	GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &UM_Map::UpdateIconPosition, 0.1f, true);
}

void UM_Map::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
	Super::NativeDestruct();
}

void UM_Map::UpdateIconPosition()
{
	if (!IsValid(MapCanvas))
	{
		UE_LOG(LogTemp, Error, TEXT("MapCanvas is NULL "));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!IsValid(PlayerPawn)) return;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();


	if (IsValid(Img_PlayerIcon))
	{
		FVector2D IconPos = WorldToMapPosition(PlayerLocation);
		if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(Img_PlayerIcon->Slot))
		{
			IconSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
			IconSlot->SetAlignment(FVector2D(0.5f, 0.5f)); 
			IconSlot->SetPosition(IconPos);
		}
		float PlayerYaw = PlayerPawn->GetActorRotation().Yaw;
		Img_PlayerIcon->SetRenderTransformAngle(PlayerYaw + YawOffset);

	}

	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), AllEnemies);

	TSet<AActor*> StillActive;

	for (AActor* Enemy : AllEnemies)
	{
		if (!IsValid(Enemy)) continue;

		float Distance = FVector::Dist(Enemy->GetActorLocation(), PlayerLocation);
		if (Distance > EnemyDetectionRadius) continue;

		StillActive.Add(Enemy);

		FVector2D IconPos = WorldToMapPosition(Enemy->GetActorLocation());

		UImage** ExistingIconPtr = EnemyIconWidgets.Find(Enemy);
		UImage* IconWidget = (ExistingIconPtr && IsValid(*ExistingIconPtr)) ? *ExistingIconPtr : nullptr;

		if (!IconWidget)
		{
			if (!IsValid(EnemyIconTexture)) continue;

			IconWidget = NewObject<UImage>(this);
			if (!IsValid(IconWidget)) continue;

			IconWidget->SetBrushFromTexture(EnemyIconTexture);
			MapCanvas->AddChildToCanvas(IconWidget);

			if (UCanvasPanelSlot* NewSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
			{
				NewSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
				NewSlot->SetAlignment(FVector2D(0.f, 0.f));
				NewSlot->SetAutoSize(false);
				NewSlot->SetSize(FVector2D(12.f, 12.f));
			}

			EnemyIconWidgets.Add(Enemy, IconWidget);
		}

		if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(IconWidget->Slot))
		{
			IconSlot->SetPosition(IconPos);
		}
	}

	TArray<AActor*> ToRemove;
	for (auto& Pair : EnemyIconWidgets)
	{
		if (!StillActive.Contains(Pair.Key))
		{
			if (IsValid(Pair.Value))
			{
				Pair.Value->RemoveFromParent();
			}
			ToRemove.Add(Pair.Key);
		}
	}
	for (AActor* Key : ToRemove)
	{
		EnemyIconWidgets.Remove(Key);
	}
}

FVector2D UM_Map::WorldToMapPosition(FVector WorldLocation) const
{
	const float MinX = -20495.23f, MaxX = 15355.93f;
	const float MinY = -12130.20f, MaxY = 11334.46f;

	float NormalizedX = 1.f - (WorldLocation.X - MinX) / (MaxX - MinX);
	float NormalizedY = (WorldLocation.Y - MinY) / (MaxY - MinY);

	FVector2D BasePos = FVector2D(NormalizedX * ImageSize.X, (1.f - NormalizedY) * ImageSize.Y);
	return BasePos + PositionOffset;
}