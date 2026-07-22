// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Blink.generated.h"

/**
 * 
 */
UCLASS()
class MYTHION_API UGA_Blink : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Blink();
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float DashStrength = 2000.f;

	
protected : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blink")
	float MaxBlinkDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float MinBlinkDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float BlinkDistancePerLevel = 10.f;

	float GetMaxBlinkDistance() const;

	UFUNCTION()
	void OnDashFinished();




	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private :

	
	FVector ValidateBlinkTarget(const FVector& RawTarget, const AActor* Avatar) const;
};
