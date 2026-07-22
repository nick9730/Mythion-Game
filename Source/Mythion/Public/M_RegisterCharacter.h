// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "M_RegisterCharacter.generated.h"

UCLASS()
class MYTHION_API AM_RegisterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AM_RegisterCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
