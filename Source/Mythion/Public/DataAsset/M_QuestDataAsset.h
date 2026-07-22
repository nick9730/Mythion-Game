
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "M_QuestDataAsset.generated.h"



class UGameplayEffect;
class AActor;

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotTaken,
    Pending,
    Completed
};


USTRUCT(BlueprintType)
struct FQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FString QuestID;

    UPROPERTY(EditAnywhere)
    FString QuestDescription;

    UPROPERTY(EditAnywhere)
    int32 RequiredKills = 0;

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> TargetEnemyClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UGameplayEffect> RewardEffect;

    UPROPERTY(EditAnywhere)
    int32 CoinReward = 0;

    UPROPERTY(BlueprintReadWrite)
    EQuestStatus Status = EQuestStatus::NotTaken;

};

UCLASS()
class MYTHION_API UM_QuestDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    TArray<FQuestData> Quests;
};