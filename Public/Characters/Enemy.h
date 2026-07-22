// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/Enemies/M_Enemy_Verticals.h"
#include "Enemy.generated.h"


class UWidgetComponent;
class UGameplayEffect;
class UBehaviorTree;

UCLASS()
class MYTHION_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleDeath() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UM_Enemy_Verticals> HealthBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FString EnemyDisplayName = TEXT("Enemy");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	float HealthBarVisibilityRadius = 1500.f;



  bool IsAlive() const { return bAlive; }
  void SetAlive(bool bNewAlive) { bAlive = bNewAlive; }

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_PlayDeathAnimation();

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_HideWeapon();


  UPROPERTY(EditAnywhere, Category = "Abilities")
  TSubclassOf<UGameplayAbility> AbilityClass;


protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffectEnemy;

private:
	bool bAlive = true;

	UPROPERTY(EditAnywhere, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;
};
