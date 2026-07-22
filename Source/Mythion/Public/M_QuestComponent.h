// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "M_QuestComponent.generated.h"



USTRUCT(BlueprintType)
struct FQuestKillProgress
{
	GENERATED_BODY()

	UPROPERTY()
	FString QuestID;

	UPROPERTY()
	int32 Kills = 0;
};





class UM_QuestDataAsset;
struct FQuestData;
class AAM_QuestNPC;
class AM_PlayerController;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FQuestData, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, FQuestData, Quest, int32, CurrentKills);




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTHION_API UM_QuestComponent : public UActorComponent
{
	GENERATED_BODY()


public:	

	UM_QuestComponent();

	UFUNCTION(Server, Reliable)
	void Server_AddQuest(FQuestData Quest);

	UFUNCTION(Server, Reliable)
	void Server_AddActiveQuest(FQuestData Quest);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<FString> CompletedQuestIDs;

	UPROPERTY(Replicated)
	TArray<FQuestData> ActiveQuestAssets;


	UPROPERTY(BlueprintAssignable)
	FOnQuestUpdated OnQuestUpdated;


	//kills
	void OnEnemyKilled(TSubclassOf<AActor> EnemyClass, AM_PlayerController* PC);

	UPROPERTY()
	TArray<FQuestKillProgress> KillProgress;





	void CompleteQuest(FQuestData Quest);
	FQuestData  FindQuestInMasterAsset(FString QuestID);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
	UM_QuestDataAsset* MasterQuestDataAsset;

	/*
	UM_QuestComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UM_QuestDataAsset* QuestDataAsset;

	UFUNCTION(Server, Reliable)
	void Server_AddQuest(FQuestData Quest);

	
	UPROPERTY()
	TArray<FQuestKillProgress> KillProgress;

	UFUNCTION(Client, Reliable)
	void Client_UpdateKills(FQuestData Quest, int32 CurrentKills);
	
	UPROPERTY()
	AAM_QuestNPC* CurrentQuestGiver;

	UFUNCTION(Client,Reliable)
	void Client_CompleteQuest(FQuestData Quest);

	UFUNCTION(Client, Reliable)
	void Client_UpdateQuestStatus(AAM_QuestNPC* QuestNPC, EQuestStatus Status);

	UPROPERTY(Replicated)
	TArray<FString> CompletedQuestIDs;
	
	

	
	UFUNCTION(Server, Reliable)
	void Server_CompleteQuest(FQuestData Quest);


	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests)
	TArray<FQuestData> ActiveQuestAssets;

	UFUNCTION()
	void OnRep_ActiveQuests();

	void OnEnemyKilled(TSubclassOf<AActor> EnemyClass);
	void AddQuest(FQuestData Quest);

	UPROPERTY(BlueprintAssignable)
	FOnQuestUpdated OnQuestUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY()
	TMap<FString, TWeakObjectPtr<AAM_QuestNPC>> QuestNPCs;


	FQuestData FindQuestInMasterAsset(FString QuestID);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quests")
	class UM_QuestDataAsset* MasterQuestDataAsset;

 // Backend
	void BackEnd_SaveQuests(FQuestData Quest);

		
*/
};
