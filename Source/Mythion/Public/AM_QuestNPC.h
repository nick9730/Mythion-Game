// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "Components/WidgetComponent.h"
#include "M_QuestStatus.h"
#include "GameFramework/Character.h"
#include "M_QuestDialolgWidget.h"
#include "AM_QuestNPC.generated.h"

class UM_QuestDataAsset;
class UStaticMeshComponent;
struct FQuestData;
class UMaterialInterface;
class AAIController;
class UWidgetComponent;
class UM_QuestStatus;
class UM_QuestDialogWidget;	
class APlayerCharacter;


UCLASS()
class MYTHION_API AAM_QuestNPC : public  ACharacter
{
	GENERATED_BODY()
	
public:	
	AAM_QuestNPC();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FString NPCQuestID;

	UPROPERTY(EditAnywhere, Category = "Quest")
	FString QuestID;


	UPROPERTY()
	APlayerCharacter* InteractingPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	UM_QuestDataAsset* QuestDataAsset;

	UPROPERTY()
	FQuestData CurrentQuestData;


	UPROPERTY(EditAnywhere, Category = "Highlight")
	UMaterialInterface* HighlightMaterial;

	UPROPERTY()
	UMaterialInterface* OriginalMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* CharacterMesh;


	// 3D Widget for Quest Status over the NPC's head
	UPROPERTY(VisibleAnywhere, Category = "UI")
	UWidgetComponent* QuestStatusIcon;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UM_QuestStatus> QuestStatusIconClass;

	UPROPERTY()
	UM_QuestStatus* QuestStatusStatusIcon;



	UPROPERTY(EditAnywhere, Category = "")
	float InteractionDistance = 800.f;

	UFUNCTION()
	void UpdateQuestStatusIcon(EQuestStatus Status);


	 void Interact(AActor* Caller) ;
	 void OnFocused() ;
	 void OnUnfocused() ;
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


};
