// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayEffect.h"
#include "ActiveGameplayEffectHandle.h"
#include "M_PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UStatsWidget;
class UM_AbilitiesContainer;
class 	UM_Inventory;
class UInv_HUDWidget;

UCLASS()
class MYTHION_API AM_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AM_PlayerController();

	FTimerHandle InventoryInitTimer;
	void TryInitInventory();




	UFUNCTION(Server, Reliable)
	void Server_DropItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_UseItem(int32 SlotIndex);


	UFUNCTION(Client, Reliable)
	void Client_OnInventoryFull();


	UFUNCTION(Client, Reliable)
	void Client_CantUseItem();

protected:
	//Initialization
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;



	UFUNCTION()
	void OnASCReady(UAbilitySystemComponent* ASC, UAttributeSet* AS);

	UPROPERTY(EditAnywhere)
	TSubclassOf<UM_AbilitiesContainer> AbilityBarClass;


	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UStatsWidget> StatsWidgetClass;

	UPROPERTY()
	UStatsWidget* StatsWidget;



	// Input Actions
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;








	//For now nothing
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;
	//

	// Inventory
	UPROPERTY(EditAnywhere)
	TSubclassOf<UM_Inventory> InventoryWidgetClass;

	UPROPERTY()
	UM_Inventory* InventoryWidget;

	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

	UPROPERTY()
	bool bInventoryOpen=false;


	UFUNCTION(Server, Reliable)
	void Server_Interact(AM_Interactive_Item* Item);
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UGameplayEffect> InventoryOpenEffect;

	FActiveGameplayEffectHandle InventoryEffectHandle;

	UFUNCTION(Server, Reliable)
	void Server_ApplyInventoryEffect(bool bOpen);

     


	UFUNCTION()
	void OnInventoryFull();


	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AM_Interactive_Item> InteractiveItemClass;


	UPROPERTY(EditDefaultsOnly, Category = "Mythion | Inventory")
	double TraceDistance;


	TWeakObjectPtr<AActor> FocusedItem;
	TWeakObjectPtr<AActor> LastFocusedItem;


	void TraceForItem();

private:

	UPROPERTY(EditDefaultsOnly, Category = "Mythion | Contexts")
	TArray<TObjectPtr<UInputMappingContext>> AddedMappingContexts;



	//Interactions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void StopJumping();
	void TryInteract();



};















