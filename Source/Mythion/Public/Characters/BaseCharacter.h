// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "BaseCharacter.generated.h"

class UM_AbilitySystemComponent;
class UM_AttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UWeaponDataASset;
class AWeaponBase;

UCLASS()
class MYTHION_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	const UM_AttributeSet* GetAttributeSet() const { return AttributeSet; }


	UPROPERTY(Replicated)
	TObjectPtr<AWeaponBase> EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UWeaponDataASset* WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName WeaponName;

	virtual void SpawnWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeAttributes(TSubclassOf<UGameplayEffect> InitEffect) const;
	virtual void HandleDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UM_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UM_AttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> InitializeAttributesEffect;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AliveChanged)
	bool bAlive = true;

	UFUNCTION()
	void OnRep_AliveChanged() {}

private:
	void OnHealthChanged(const FOnAttributeChangeData& Data);
};

