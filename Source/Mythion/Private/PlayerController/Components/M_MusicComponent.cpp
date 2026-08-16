// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_MusicComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/Components/M_MusicComponent.h"

UM_MusicComponent::UM_MusicComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UM_MusicComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UM_MusicComponent::PlayTheSound()
{
    AActor *OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
        return;

    AActor *LocationSource = OwnerActor;
    if (AController *AsController = Cast<AController>(OwnerActor))
    {
        LocationSource = AsController->GetPawn();
    }
    if (!IsValid(LocationSource))
        return;

    if (IsValid(GenericSoundAudioComponent))
    {
        GenericSoundAudioComponent->OnAudioFinished.RemoveDynamic(this, &UM_MusicComponent::OnFinishingGenericSound);
        GenericSoundAudioComponent->FadeOut(0.3f, 0.0f);
        GenericSoundAudioComponent = nullptr;
    }

    if (!GetSpotted())
    {
        if (GenericGameSounds.IsValidIndex(SoundIndex))
        {
            GenericSoundAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
                this, GenericGameSounds[SoundIndex], LocationSource->GetActorLocation(), FRotator::ZeroRotator, 0.3f);

            if (GenericSoundAudioComponent)
            {
                GenericSoundAudioComponent->FadeIn(0.3f, 1.0f);
                GenericSoundAudioComponent->OnAudioFinished.AddDynamic(this,
                                                                       &UM_MusicComponent::OnFinishingGenericSound);
            }
        }
    }
    else
    {
        if (SoundCombat.IsValidIndex(SoundIndex))
        {
            GenericSoundAudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, SoundCombat[SoundIndex],
                                                                                LocationSource->GetActorLocation());

            if (GenericSoundAudioComponent)
            {
                GenericSoundAudioComponent->FadeIn(0.3f, 1.0f);
                GenericSoundAudioComponent->OnAudioFinished.AddDynamic(this,
                                                                       &UM_MusicComponent::OnFinishingGenericSound);
            }
        }
    }
}

void UM_MusicComponent::OnFinishingGenericSound()
{
    const TArray<USoundBase *> &ActiveSoundArray = GetSpotted() ? SoundCombat : GenericGameSounds;

    if (ActiveSoundArray.Num() > 1)
    {
        int32 NewIndex;
        do
        {
            NewIndex = FMath::RandRange(0, ActiveSoundArray.Num() - 1);
        } while (NewIndex == SoundIndex);

        SoundIndex = NewIndex;
    }
    else if (ActiveSoundArray.Num() == 1)
    {
        SoundIndex = 0;
    }

    PlayTheSound();
}

void UM_MusicComponent::NotifyEnemyPerception(bool bPerceived, AActor *Enemy)
{
    bool bWasSpotted = GetSpotted();

    if (bPerceived)
    {
        EnemiesSpottingMe.AddUnique(Enemy);
    }
    else
    {
        EnemiesSpottingMe.Remove(Enemy);
    }

    bool bIsSpottedNow = GetSpotted();

    if (bWasSpotted == bIsSpottedNow)
        return;

    if (!IsValid(GetOwner()))
        return;

    if (bIsSpottedNow)
    {
        GetOwner()->GetWorldTimerManager().ClearTimer(MusicDebounceTimer);
        SoundIndex = 0;
        PlayTheSound();
    }
    else
    {
        GetOwner()->GetWorldTimerManager().ClearTimer(MusicDebounceTimer);
        GetOwner()->GetWorldTimerManager().SetTimer(
            MusicDebounceTimer,
            [this]() {
                SoundIndex = 0;
                PlayTheSound();
            },
            MusicDebounceDelay, false);
    }
}