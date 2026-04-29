// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "M_Enemy_Verticals.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;

UCLASS()
class MYTHION_API UM_Enemy_Verticals : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWidget(UAbilitySystemComponent* ASC, const FString& EnemyName, int32 EnemyLevel);
	UFUNCTION(BlueprintCallable)
	void SetOwningEnemy(AEnemy* Enemy);

protected:
	virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

private:
	void BindToASC(UAbilitySystemComponent* ASC);
	void UnbindFromASC();

	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void UpdateHealthBar();

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	FDelegateHandle HealthHandle;
	FDelegateHandle MaxHealthHandle;

	UPROPERTY()
	TObjectPtr<AEnemy> OwningEnemy;


	float CurrentHealth = 0.f;
	float CurrentMaxHealth = 1.f;
};