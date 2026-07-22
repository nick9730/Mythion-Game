// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "DataAsset/CharacterClasses.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "DataAsset/M_CommonAbilities.h"
#include "Simple_Inventory/Widgets/M_PreviewActorInventory.h"

#include "PlayerCharacter.generated.h"

USTRUCT(BlueprintType)
struct FCommonAbilityEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditAnywhere)
	int32 InputID;
};

class USpringArmComponent;
class UCameraComponent;
class AM_PlayerState;
class UCharacterClasses;
struct FOnAttributeChangeData;
struct FItemData;
class UM_QuestComponent;
class  UAbilitySystemComponent;
class UM_AttributeSet;
class UWidgetComponent;
class UM_CommonAbilities;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnASCInitialized, UAbilitySystemComponent*, ASC, UAttributeSet*, AS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityGranted, FGrantAbilitiesDataByLevel, AbilityEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityInitialized, FGrantAbilitiesDataByLevel, AbilityEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, float, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreviewReady, UTextureRenderTarget2D*, RT);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniMapReady, UTextureRenderTarget2D*, RenderTarget);






UCLASS()
class MYTHION_API APlayerCharacter : public ABaseCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	UAttributeSet* GetAttributeSet() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return Camera; }



	//name
	UPROPERTY(VisibleAnywhere, Category = "UI")
	UWidgetComponent* NameplateComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> NameplateWidgetClass;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerName)
	FString PlayerUsername;

	UFUNCTION()
	void OnRep_PlayerName();
	

	UPROPERTY(Replicated)
	FVector DeathLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS|Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<const class UM_AttributeSet> AttributeSet;

	UFUNCTION()
	void LevelUp(float NewLevel);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS")
	UAbilitySystemComponent* GetASC() const { return GetAbilitySystemComponent(); }

	UPROPERTY(BlueprintAssignable)
	FOnASCInitialized OnASCInitialized;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class UInventoryComponent* InventoryComponent;


	UFUNCTION(Server, Reliable)
	void Server_ApplyCharacterClass(FGameplayTag ClassTag);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyCharacterClass(FGameplayTag ClassTag);

	//Player State CharacterData
	void ApplyCharacterClassData(TSoftObjectPtr<UCharacterClasses> ClassData, const FGameplayTag TagMatches);

	UFUNCTION(Server, Reliable)
	void Server_DestroyEquippedItems();


	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LoadingScreenClass;

	bool bASCInitialized = false;


	UPROPERTY()
	UUserWidget* LoadingScreenWidget;

	void SetupAttributes();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> InitStatsEffect;

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

	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> LevelUpStats;


	//Abilities related
	void GrantAbilities();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class")
	UCharacterClasses* CharacterClassData;



	UFUNCTION(Client, Reliable)
	void Client_OnAbilityGranted(FGrantAbilitiesDataByLevel AbilityEntry);



	UPROPERTY(BlueprintAssignable)
	FOnAbilityGranted OnAbilityGranted;


	UPROPERTY(BlueprintAssignable)
	FOnLevelChanged OnLevelChanged;



	UPROPERTY(BlueprintAssignable)
	FOnAbilityInitialized OnAbilityInitialized;


	UPROPERTY(ReplicatedUsing = OnRep_PlayerClassTag, BlueprintReadOnly)
	FGameplayTag PlayerClassTag;

	UFUNCTION()
	void OnRep_PlayerClassTag();

	UFUNCTION()
	void OnDeathTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Death")
	void HandleDeath();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void HandleRespawn();



	//Inventory related

	UPROPERTY()
	AM_PreviewActorInventory* PreviewActor;

	UPROPERTY(EditAnywhere, Category = "Preview")
	TSubclassOf<UAnimInstance> PreviewAnimClass;


	UPROPERTY(BlueprintAssignable)
	FOnPreviewReady OnPreviewReady;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	UM_CommonAbilities* CommonAbilitiesData;

	UFUNCTION()
	void GrantCommonAbilities();

	//Quests
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	UM_QuestComponent* QuestComponent;

	//Warrior 

	UPROPERTY()
	FActiveGameplayEffectHandle ActiveWeaponEffectHandle;
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveArmorEffectHandle;


	UFUNCTION()
	void UnequipWeapon(FItemData WeaponItem);


	UFUNCTION()
	void OnWeaponEquipped(FItemData Item);

	UFUNCTION(Server, Reliable)
	void Server_SpawnAndEquipWeapon(FItemData Item);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachWeapon(AWeaponBase* Weapon, FItemData Item);



	//MiniMap
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	class USceneCaptureComponent2D* MiniMapCaptureComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	class UTextureRenderTarget2D* MiniMapRenderTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	int32 MiniMapResolution = 512;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float MiniMapOrthoWidth = 5000.f;

	UPROPERTY(BlueprintAssignable, Category = "Minimap")
	FOnMiniMapReady OnMiniMapReady;


	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	class UMaterialInterface* MiniMapBaseMaterial;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	class UMaterialInstanceDynamic* MiniMapDynamicMaterial;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	FVector2D GetFullMapIconPosition(FVector2D ImageSize) const;




protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	

	FTimerHandle AutoSaveTimer;
};
