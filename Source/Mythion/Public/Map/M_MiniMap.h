#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/M_MerchantNPC.h"
#include "AM_QuestNPC.h"
#include "M_MiniMap.generated.h"

class UImage;
class UCanvasPanel;
class UTexture2D;
class AEnemy;

UCLASS()
class MYTHION_API UM_MiniMap : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// Το εξωτερικό, clipped container (πρέπει να έχει Clip to Bounds = true στο Designer)
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RotatingCanvas;

	// Background μέσα στο περιστρεφόμενο canvas
	UPROPERTY(meta = (BindWidget))
	UImage* Img_Background;

	// Player icon, ΕΞΩ από το RotatingCanvas, πάντα στο κέντρο, πάντα σταθερό (δείχνει πάνω)
	UPROPERTY(meta = (BindWidget))
	UImage* Img_PlayerIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* BackgroundTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* EnemyIconTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* QuestIconTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	UTexture2D* MerchantIconTexture;

	// Μέγεθος του παραθύρου minimap σε pixels (π.χ. 180x180)
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float WindowSize = 180.f;

	// Πόσα world units φαίνονται από το κέντρο μέχρι την άκρη του κύκλου
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float WorldRadius = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float DetectionRadius = 5000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float RotationOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TSubclassOf<class AAM_QuestNPC> QuestNPCClass;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TSubclassOf<class AM_MerchantNPC> MerchantNPCClass;


	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float YawOffset;


private:
	FTimerHandle UpdateTimer;

	UPROPERTY()
	TMap<AActor*, UImage*> EnemyIconWidgets;

	UPROPERTY()
	TMap<AActor*, UImage*> QuestIconWidgets;

	UPROPERTY()
	TMap<AActor*, UImage*> MerchantIconWidgets;

	void UpdateMiniMap();

	FVector2D WorldToScaledPixel(FVector WorldLocation) const;

	void UpdateIconsForClass(TSubclassOf<AActor> ActorClass, UTexture2D* IconTexture,
		TMap<AActor*, UImage*>& IconMap, FVector PlayerLocation);
};