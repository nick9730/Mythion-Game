// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "M_QuestUIComponent.generated.h"

class AAM_QuestNPC;
class UM_QuestDialolgWidget;
class UM_CompletedQuestWidget;
class UGameplayEffect;
struct FQuestData;
enum class EQuestStatus : uint8;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_QuestUIComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_QuestUIComponent();

    UFUNCTION(Client, Reliable)
    void Client_ShowQuestDialog(AAM_QuestNPC *NPC, FQuestData QuestData);

    UFUNCTION(Client, Reliable)
    void Client_PendingQuestLoaded(FQuestData QuestAsset);

    UFUNCTION(Server, Reliable)
    void Server_CompleteQuest(FQuestData QuestAsset);

    UFUNCTION(Client, Reliable)
    void Client_CompleteQuest(FQuestData QuestAsset);

    UFUNCTION(Client, Reliable)
    void Client_UpdateKills(FQuestData Quest, int32 CurrentKills);

    void UpdateNPCQuestStatus(FQuestData Quest, EQuestStatus Status);
    void ShowCompletedQuest(FQuestData Quest);

    UPROPERTY(EditDefaultsOnly, Category = "Quest UI")
    TSubclassOf<UM_QuestDialolgWidget> QuestWidgetDialogClass;

    UPROPERTY(EditDefaultsOnly, Category = "Quest UI")
    TSubclassOf<UM_CompletedQuestWidget> QuestCompletedClass;

    UPROPERTY(EditDefaultsOnly, Category = "Economy")
    TSubclassOf<UGameplayEffect> CoinEffect;

    UPROPERTY()
    TObjectPtr<UM_QuestDialolgWidget> QuestWidgetDialog;

    UPROPERTY()
    TObjectPtr<UM_CompletedQuestWidget> QuestCompletedWidget;

  private:
};
