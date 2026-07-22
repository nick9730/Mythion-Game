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
class UCanvasPanel;
class APlayerCharacter;
class UM_ChatWidget;
class UM_QuestsInfos;

UCLASS()
class MYTHION_API UStatsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeWidget(UAbilitySystemComponent* ASC);
    UPROPERTY(meta = (BindWidget))
    UM_QuestsInfos* QuestInfoWidget;
    

protected:
    virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;
   virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ManaBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> EnergyBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> EnergyText;

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

    UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> EnemyDotsContainer;



    UPROPERTY(meta = (BindWidget),BlueprintReadOnly)
    UM_ChatWidget* ChatWidget;

    UFUNCTION(BlueprintCallable)
    void FocusChatInput();

    UFUNCTION(BlueprintCallable)
    void UnFocusChatInput();

    FTimerHandle BindASCTimerHandle;

    UFUNCTION()
    void TryBindASC();

    FVector2d GetMiniMapPosition(APlayerCharacter* Player, AActor* Target, float MinimapSize, float CaptureRange);

private:
    void BindToASC(UAbilitySystemComponent* ASC);


    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
    void OnManaChanged(const FOnAttributeChangeData& Data);
    void OnMaxManaChanged(const FOnAttributeChangeData& Data);
    void OnXPChanged(const FOnAttributeChangeData& Data);
    void OnXPMaxChanged(const FOnAttributeChangeData& Data);
    void OnLevelChanged(const FOnAttributeChangeData& Data);
    void OnEnergyChanged(const FOnAttributeChangeData& Data);
    void OnMaxEnergyChanged(const FOnAttributeChangeData& Data);


    void UpdateHealthBar();
    void UpdateManaBar();
    void UpdateXPBar();
    void UpdateEnergyBar();


    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> BoundASC;

    FDelegateHandle HealthHandle;
    FDelegateHandle MaxHealthHandle;
    FDelegateHandle ManaHandle;
    FDelegateHandle MaxManaHandle;
    FDelegateHandle XPHandle;
    FDelegateHandle XPMaxHandle;
    FDelegateHandle LevelHandle;
    FDelegateHandle EnergyHandle;
    FDelegateHandle MaxEnergyHandle;





    float CurrentHealth = 0.f;
    float CurrentMaxHealth = 1.f;
    float CurrentMana = 0.f;
    float CurrentMaxMana = 1.f;
    float CurrentXP = 0.f;
    float CurrentXPMax = 1.f;
    float CurrentLevel = 1.f;
    float CurrentEnergy = 0.f;
    float CurrentMaxEnergy = 1.f;

};
