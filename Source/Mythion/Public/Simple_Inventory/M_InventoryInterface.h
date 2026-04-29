// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "M_InventoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UM_InventoryInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MYTHION_API IM_InventoryInterface
{
	GENERATED_BODY()
	
	
public:

		virtual void Interact(AActor* PlayerChar) = 0;
		virtual void OnFocused() = 0;
		virtual void OnUnfocused() = 0;
};
