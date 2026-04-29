#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "StatsWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
struct FGameplayEventData;
struct FGameplayTag;

UCLASS()
class MYTHION_API UStatsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeWidget(UAbilitySystemComponent* ASC);
    

protected:
    virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ManaBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> XPBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ManaText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> LevelText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> XPText;

private:
    void BindToASC(UAbilitySystemComponent* ASC);


    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
    void OnManaChanged(const FOnAttributeChangeData& Data);
    void OnMaxManaChanged(const FOnAttributeChangeData& Data);
    void OnXPChanged(const FOnAttributeChangeData& Data);
    void OnXPMaxChanged(const FOnAttributeChangeData& Data);
    void OnLevelChanged(const FOnAttributeChangeData& Data);

    void UpdateHealthBar();
    void UpdateManaBar();
    void UpdateXPBar();

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> BoundASC;

    FDelegateHandle HealthHandle;
    FDelegateHandle MaxHealthHandle;
    FDelegateHandle ManaHandle;
    FDelegateHandle MaxManaHandle;
    FDelegateHandle XPHandle;
    FDelegateHandle XPMaxHandle;
    FDelegateHandle LevelHandle;




    float CurrentHealth = 0.f;
    float CurrentMaxHealth = 1.f;
    float CurrentMana = 0.f;
    float CurrentMaxMana = 1.f;
    float CurrentXP = 0.f;
    float CurrentXPMax = 1.f;
    float CurrentLevel = 1.f;
};
