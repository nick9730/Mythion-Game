// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "GAS/M_AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"

AM_PlayerState::AM_PlayerState()
{
    // ASC on PlayerState - owner = PlayerState, avatar = Character
    AbilitySystemComponent = CreateDefaultSubobject<UM_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

 
    AttributeSet = CreateDefaultSubobject<UM_AttributeSet>(TEXT("AttributeSet"));


}

UAbilitySystemComponent* AM_PlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

