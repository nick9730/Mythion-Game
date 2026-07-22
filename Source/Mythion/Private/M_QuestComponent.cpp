

#include "M_QuestComponent.h"
#include "DataAsset/M_QuestDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "M_PlayerController.h"
#include "M_QuestsInfos.h"
#include  "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h" 
#include "Attributes/M_AttributeSet.h"
#include   "AM_QuestNPC.h"


UM_QuestComponent::UM_QuestComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);

}

void UM_QuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UM_QuestComponent, ActiveQuestAssets, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UM_QuestComponent, CompletedQuestIDs, COND_None, REPNOTIFY_Always);
}

void UM_QuestComponent::Server_AddQuest_Implementation(FQuestData Quest)
{
    Quest.Status = EQuestStatus::Pending;
    ActiveQuestAssets.Add(Quest);
    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!IsValid(PlayerCharacter)) return;
    AM_PlayerController* PC = Cast<AM_PlayerController>(PlayerCharacter->GetController());
    if (!IsValid(PC)) return;
    PC->Client_PendingQuestLoaded(Quest);
//	PC->SavePlayerQuestsForBackEnd(Quest);
}


void UM_QuestComponent::OnEnemyKilled(TSubclassOf<AActor> EnemyClass,AM_PlayerController* PC)
{


    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!IsValid(PlayerCharacter)) return;


    for (FQuestData& Quest : ActiveQuestAssets)
    {

        if (Quest.Status != EQuestStatus::Pending) continue;
        
    
        if (!EnemyClass->IsChildOf(Quest.TargetEnemyClass)) continue;
      

        int32 FoundIndex = KillProgress.IndexOfByPredicate([&](const FQuestKillProgress& K) { return K.QuestID == Quest.QuestID; });
        if (FoundIndex == INDEX_NONE)
        {
            FQuestKillProgress New;
            New.QuestID = Quest.QuestID;
            New.Kills = 0;
            FoundIndex = KillProgress.Add(New);
        }

        if (KillProgress[FoundIndex].Kills >= Quest.RequiredKills) continue;
        KillProgress[FoundIndex].Kills++;
      PC->Client_UpdateKills(Quest, KillProgress[FoundIndex].Kills);

        if (KillProgress[FoundIndex].Kills >= Quest.RequiredKills)
        {
          
		  CompleteQuest(Quest);
		  
		
        }
        else
        {
            Quest.Status = EQuestStatus::Pending;
         
        }
    }

}




void UM_QuestComponent::CompleteQuest(FQuestData Quest)
{
	Quest.Status = EQuestStatus::Completed;
    ActiveQuestAssets.RemoveAll([&](const FQuestData& Q) { return Q.QuestID == Quest.QuestID; });

    CompletedQuestIDs.Add(Quest.QuestID);

    APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!IsValid(PlayerCharacter)) return;
    AM_PlayerController* PC = Cast<AM_PlayerController>(PlayerCharacter->GetController());
    if (!IsValid(PC))
    {
        return;
    }
	PC->Client_CompleteQuest(Quest);
    PC->Server_CompleteQuest(Quest);

}

FQuestData  UM_QuestComponent::FindQuestInMasterAsset(FString QuestID)
{
    if (!IsValid(MasterQuestDataAsset)) return FQuestData();

    for (const FQuestData& Quest : MasterQuestDataAsset->Quests)
    {
        if (Quest.QuestID == QuestID)
            return Quest;
    }
    return FQuestData();
}

void UM_QuestComponent::Server_AddActiveQuest_Implementation(FQuestData Quest)
{
    ActiveQuestAssets.Add(Quest);
}

