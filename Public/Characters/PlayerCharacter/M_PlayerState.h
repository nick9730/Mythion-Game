// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "M_PlayerState.generated.h"

class UM_AbilitySystemComponent;
class UM_AttributeSet;

UCLASS()
class MYTHION_API AM_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AM_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UM_AttributeSet* GetAttributeSet() const { return AttributeSet; }




protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UM_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UM_AttributeSet> AttributeSet;


};



















