// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "M_QuestController.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API AM_QuestController : public AAIController
{
	GENERATED_BODY()

public:


	virtual void OnPossess(APawn* InPawn) override;
	
};
