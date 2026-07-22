// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "DataAsset/CharacterClasses.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

#include "PlayerCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class AM_PlayerState;
class UCharacterClasses;
struct FOnAttributeChangeData;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnASCInitialized, UAbilitySystemComponent*, ASC, UAttributeSet*, AS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityGranted, FGrantAbilitiesDataByLevel, AbilityEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityInitialized, FGrantAbilitiesDataByLevel, AbilityEntry);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, float, NewLevel);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewReady, UTextureRenderTarget2D*, RT);






UCLASS()
class MYTHION_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	UAttributeSet* GetAttributeSet() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return Camera; }


	UFUNCTION() 
	void LevelUp(float NewLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS")
	UAbilitySystemComponent* GetASC() const { return GetAbilitySystemComponent(); }

	UPROPERTY(BlueprintAssignable)
	FOnASCInitialized OnASCInitialized;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;




	//Player State CharacterData
	void ApplyCharacterClassData(TSoftObjectPtr<UCharacterClasses> ClassData, const FGameplayTag TagMatches);
	void SetupAttributes();


	void OnLevelAttributeChanged(const FOnAttributeChangeData& Data);


	//XP and Level up related

	UPROPERTY(EditAnywhere, Category = "XP")
	UCurveTable* XpScaleTable;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<UGameplayEffect> LevelUpEffect;


	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<UGameplayEffect> XpMaxEffect;


	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<UGameplayEffect> SetXpCorrectly;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<UGameplayEffect> LevelUpStats;

	//Abilities related
	void GrantAbilities();

	UPROPERTY(EditAnywhere, Category = "Character Class")
	UCharacterClasses* CharacterClassData;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnAbilityGranted(FGrantAbilitiesDataByLevel AbilityEntry);

	UPROPERTY()
	FGameplayTag PlayerClassTag = FGameplayTag::RequestGameplayTag(FName("Classes.Warrior"));

	UPROPERTY(BlueprintAssignable)
	FOnAbilityGranted OnAbilityGranted;


	UPROPERTY(BlueprintAssignable)
	FOnLevelChanged OnLevelChanged;



	UPROPERTY(BlueprintAssignable)
	FOnAbilityInitialized OnAbilityInitialized;


	//Inventory related
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UTextureRenderTarget2D> PreviewRenderTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> PreviewCapture;


	UPROPERTY(BlueprintAssignable)
	FOnPreviewReady OnPreviewReady;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
};
