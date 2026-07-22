// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "M_LevelUpExecution.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API UM_LevelUpExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UM_LevelUpExecution();
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
