// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "M_AbilitySlot.generated.h"


class UAbilitySystemComponent;

/**
 * 
 */
class UImage;
class UProgressBar;
class UTexture2D;
class UTextBlock;
struct FGameplayTag;
class UGameplayAbility;
struct FGameplayEventData;
struct FTimerHandle;

UCLASS()
class MYTHION_API UM_AbilitySlot : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UImage* AbilityIcon;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* CooldownBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KeybindText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CooldownText;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTag AbilityTag;

    UPROPERTY(BlueprintReadWrite)
    FGameplayTag CooldownTag;

    UPROPERTY()
    UAbilitySystemComponent* BoundASC;

    void SetAbility(FGameplayTag Tag, FGameplayTag Cooldown, UTexture2D* Icon, FString Keybind, UAbilitySystemComponent* ASC);

    UFUNCTION(BlueprintCallable)
    void UpdateCooldown();

	FTimerHandle CooldownUpdateTimerHandle;


protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};