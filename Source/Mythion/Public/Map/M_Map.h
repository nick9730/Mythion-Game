// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_Map.generated.h"

class UImage;
class UCanvasPanel;
class UTexture2D;

UCLASS()
class MYTHION_API UM_Map : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	FVector2D WorldToMapPosition(FVector WorldLocation) const;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Img_Background;

	UPROPERTY(meta = (BindWidget))
	UImage* Img_PlayerIcon;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MapCanvas;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	FVector2D ImageSize = FVector2D(1242.f, 870.f);

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	FVector2D PositionOffset = FVector2D(0.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* FullMapBackgroundTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* EnemyIconTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float YawOffset;

	// Μόνο enemies μέσα σε αυτή την ακτίνα (world units) εμφανίζονται στ\bο map
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float EnemyDetectionRadius = 500.f;

	UPROPERTY()
	TMap<AActor*, UImage*> EnemyIconWidgets;

private:
	FTimerHandle UpdateTimer;
	void UpdateIconPosition();
};