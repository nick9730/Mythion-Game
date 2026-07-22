// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_RespawnWidget.generated.h"

/**
 * 
 */
class UButton;

UCLASS()
class MYTHION_API UM_RespawnWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* RespawnButton;

    UFUNCTION()
    void OnRespawnClicked();

    virtual void NativeConstruct() override;
    
};
