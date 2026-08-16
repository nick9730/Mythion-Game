// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "M_MusicComponent.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class MYTHION_API UM_MusicComponent : public UActorComponent
{
    GENERATED_BODY()

  public:
    UM_MusicComponent();

    void PlayTheSound();
    void NotifyEnemyPerception(bool bPerceived, AActor *Enemy);
    bool GetSpotted() const
    {
        return EnemiesSpottingMe.Num() > 0;
    }

    UPROPERTY(EditAnywhere)
    TArray<USoundBase *> GenericGameSounds;

    UPROPERTY(EditAnywhere)
    TArray<USoundBase *> SoundCombat;

    UPROPERTY(EditAnywhere)
    float MusicDebounceDelay = 3.0f;

    UPROPERTY()
    TObjectPtr<UAudioComponent> GenericSoundAudioComponent;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> EnemiesSpottingMe;

    int32 SoundIndex = 0;

    FTimerHandle MusicDebounceTimer;

  protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnFinishingGenericSound();

  private:
};
