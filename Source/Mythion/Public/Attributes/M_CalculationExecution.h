// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "M_CalculationExecution.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API UM_CalculationExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

  
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
