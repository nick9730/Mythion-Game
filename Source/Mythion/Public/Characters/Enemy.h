// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/Enemies/M_Enemy_Verticals.h"
#include "Characters/Enemies/EnemyTypes.h"
#include "Enemy.generated.h"


class UWidgetComponent;
class UGameplayEffect;
class UBehaviorTree;
class UAbilitySystemComponent;
class AM_Enemy_Area_Spawner;
class AM_PlayerController;

UCLASS()
class MYTHION_API AEnemy : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const class UM_AttributeSet> AttributeSet;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;


	UPROPERTY( EditAnywhere, Category = "Enemy Setup Checklist | Speed")
	float WalkSpeed;

	UPROPERTY( EditAnywhere, Category = "Enemy Setup Checklist | Speed")
	float ChaseSpeed;


	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist | AIPerception")
	float SightRadius;


	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist | AIPerception")
	float LoseSightRadius;


	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist | AIPerception")
	float PeripheralVisionAngleDegrees;

	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist | AIPerception")
	float HearingRange;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy Setup Checklist")
	TObjectPtr<AM_Enemy_Area_Spawner> OwningSpawner;

	void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void HandleDeath() override;

	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist")
	FName EnemyName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	TSubclassOf<UM_Enemy_Verticals> HealthBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	FString EnemyDisplayName = TEXT("Enemy");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	float HealthBarVisibilityRadius = 500.f;

	UFUNCTION()
	void SpawnXpReward();

	UFUNCTION()
	void SpawnActor();

	FVector SpawnPoint;
	FTimerHandle RespawnTimer;

	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist")
	TSubclassOf<AActor> XpRewardClass;

	UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist")
	EEnemyType EnemyType;

	UFUNCTION()
	void InitializeEnemyAttributes(TSubclassOf<UGameplayEffect> InitEffect);

  bool IsAlive() const { return bAlive; }
  void SetAlive(bool bNewAlive) { bAlive = bNewAlive; }

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_PlayDeathAnimation();

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_HideWeapon();


  UPROPERTY(EditAnywhere, Category = "Enemy Setup Checklist")
  TArray<TSubclassOf<UGameplayAbility>> AbilityClasses;

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_DisableCollisionOnDeath();

  UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Enemy Setup Checklist ")
  TObjectPtr<UAnimMontage> DeathMontage;

  UFUNCTION()
 void FindTheCorrectEnemy();

 UPROPERTY()
 AM_PlayerController* LastDamageInstigator;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Setup Checklist")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffectEnemy;

private:
	bool bAlive = true;






};
