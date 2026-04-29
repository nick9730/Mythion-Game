#include "M_PlayerController.h"
#include "Widgets/PlayerWidget/StatsWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "InputMappingContext.h"
#include "Components/Widget.h"
#include  "AbilitySystemBlueprintLibrary.h"
#include "Widgets/M_AbilitiesContainer.h"
#include "DataAsset/CharacterClasses.h"
#include "Simple_Inventory/Widgets/M_Inventory.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Simple_Inventory/Data/M_Item_Details.h"
#include "Attributes/M_AttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"




//Initialization
AM_PlayerController::AM_PlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AM_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	

	APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn());
	if (IsValid(PC))
	{
		PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
		PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);

		UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
		if (IsValid(ASC))
		{
			OnASCReady(ASC, PC->GetAttributeSet());
		}
	}

}
void AM_PlayerController::Tick(float DeltaSeconds)
{
	TraceForItem();
}
void AM_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Context : AddedMappingContexts)
		{
			if (IsValid(Context))
			{
				Subsystem->AddMappingContext(Context, 0);
			}
		}
	}
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!IsValid(EIC)) return;

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Move);
	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AM_PlayerController::Look);
	EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &AM_PlayerController::Jump);
	EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &AM_PlayerController::StopJumping);
	EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AM_PlayerController::ToggleInventory);
	EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AM_PlayerController::TryInteract);


}
void AM_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	APlayerCharacter* PC = Cast<APlayerCharacter>(InPawn);
	if (!IsValid(PC) || !IsLocalPlayerController()) return;


	PC->OnASCInitialized.RemoveDynamic(this, &AM_PlayerController::OnASCReady);
	PC->OnASCInitialized.AddDynamic(this, &AM_PlayerController::OnASCReady);
	PC->InventoryComponent->OnInventoryFull.AddDynamic(this, &AM_PlayerController::OnInventoryFull);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			FString::Printf(TEXT("OnInventoryFull bound: %s"),
				PC->InventoryComponent->OnInventoryFull.IsBound() ? TEXT("YES") : TEXT("NO")));

	UAbilitySystemComponent* ASC = PC->GetAbilitySystemComponent();
	if (IsValid(ASC))
	{
		OnASCReady(ASC, PC->GetAttributeSet());
	}

	if (IsValid(InventoryWidgetClass))
	{
		InventoryWidget = CreateWidget<UM_Inventory>(this, InventoryWidgetClass);
		if (IsValid(InventoryWidget))
		{
			InventoryWidget->InitializeInventory(PC->InventoryComponent);
			InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
			InventoryWidget->AddToViewport();
		}
	}

}
void AM_PlayerController::OnASCReady(UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	if (!IsLocalController()) return;
	if (!IsValid(ASC)) return;

	if (IsValid(StatsWidgetClass) && !IsValid(StatsWidget))
	{
		StatsWidget = CreateWidget<UStatsWidget>(this, StatsWidgetClass);
		if (IsValid(StatsWidget))
		{
			StatsWidget->AddToViewport();
			StatsWidget->InitializeWidget(ASC);
		}
	}



}


//Actions
void AM_PlayerController::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;
	if (bInventoryOpen) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}
void AM_PlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();
	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}
void AM_PlayerController::Jump()
{
	if (!IsValid(GetCharacter())) return;
	GetCharacter()->Jump();
}
void AM_PlayerController::StopJumping()
{
	if (!IsValid(GetCharacter())) return;
	GetCharacter()->StopJumping();
}
void AM_PlayerController::TryInteract()
{
	if (!FocusedItem.IsValid()) return;



	AM_Interactive_Item* Item = Cast<AM_Interactive_Item>(FocusedItem.Get());




	if (!FocusedItem.IsValid()) return;



	Server_Interact(Item);

}
void AM_PlayerController::ToggleInventory()
{

	if (!IsLocalController()) return;
	if (!IsValid(InventoryWidget)) return;

	bool bIsVisible = InventoryWidget->GetVisibility() == ESlateVisibility::Visible;
	APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn());
	if (!IsValid(PC)) return;

	AM_PlayerState* PS = Cast<AM_PlayerState>(PC->GetPlayerState());
	if (!IsValid(PS)) return;
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();

	if (!IsValid(ASC)) return;

	if (bIsVisible)
	{
		Server_ApplyInventoryEffect(false);
		
		bInventoryOpen = false;
		GetCharacter()->Tags.Remove(FName("InventoryClose"));
		InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
		ASC->RemoveActiveGameplayEffect(InventoryEffectHandle);

	}
	else
	{
		Server_ApplyInventoryEffect(true);
		bInventoryOpen = true;
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		
	}
}





void AM_PlayerController::OnInventoryFull()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Inventory is full!"));
}



//Inventory & Interaction
void AM_PlayerController::TryInitInventory()
{
	APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn());
	if (!IsValid(PC) || !IsValid(PC->InventoryComponent)) return;

	GetWorld()->GetTimerManager().ClearTimer(InventoryInitTimer);

	if (IsValid(InventoryWidgetClass))
	{
		InventoryWidget = CreateWidget<UM_Inventory>(this, InventoryWidgetClass);
		if (IsValid(InventoryWidget))
		{
			InventoryWidget->InitializeInventory(PC->InventoryComponent);
			InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
			InventoryWidget->AddToViewport();
		}
	}
}

void AM_PlayerController::Server_DropItem_Implementation(int32 SlotIndex)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn());
	if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent)) return;


	FItemData DroppedItem = PlayerChar->InventoryComponent->Items[SlotIndex];
	if (!DroppedItem.IsValid()) return;

	PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
	


	for (int32 i = 0; i < DroppedItem.Quantity; i++)
	{
		FVector DropLocation = PlayerChar->GetActorLocation() +
			PlayerChar->GetActorForwardVector() * 100.f +
			FVector(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 0.f);

		AM_Interactive_Item* 
			
			
			ed = AM_Interactive_Item::SpawnItem(
			GetWorld(), DroppedItem.ItemDataAsset, 1, DropLocation, InteractiveItemClass
		);
	}


}
void AM_PlayerController::Server_Interact_Implementation(AM_Interactive_Item* Item)
{
	if (!IsValid(Item)) return;
	Item->Interact(GetPawn());
}
void AM_PlayerController::Server_UseItem_Implementation(int32 SlotIndex)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn());
	if (!IsValid(PlayerChar) || !IsValid(PlayerChar->InventoryComponent)) return;



	FItemData& Item = PlayerChar->InventoryComponent->Items[SlotIndex];
	if (!Item.IsValid() || !IsValid(Item.ItemDataAsset)) return;
	if (!IsValid(Item.ItemDataAsset->UseEffect)) return;

	UAbilitySystemComponent* AbilitySC = PlayerChar->GetAbilitySystemComponent();
	if (!IsValid(AbilitySC)) return;

	if (Item.ItemDataAsset->ItemStats == EItemStats::HealthRefill)
	{
		float CurrentHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetHealthAttribute());
		float MaxHealth = AbilitySC->GetNumericAttribute(UM_AttributeSet::GetMaxHealthAttribute());
		if (CurrentHealth >= MaxHealth)
		{
			Client_CantUseItem();
			return;
		}

		FItemData& ItemSlot = PlayerChar->InventoryComponent->Items[SlotIndex];
		if (!ItemSlot.IsValid() || !IsValid(ItemSlot.ItemDataAsset)) return;
		if (!IsValid(ItemSlot.ItemDataAsset->UseEffect)) return;



		// Apply effect
		UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();
		if (!IsValid(ASC)) return;

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(ItemSlot.ItemDataAsset->UseEffect, 1, Context);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

		// Αφαίρεσε 1 quantity
		ItemSlot.Quantity--;
		if (ItemSlot.Quantity <= 0)
			PlayerChar->InventoryComponent->Server_RemoveItem(SlotIndex);
		else
			PlayerChar->InventoryComponent->OnInventoryChanged.Broadcast();
	}
}
void AM_PlayerController::Client_OnInventoryFull_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Inventory is Full!"));
}
void AM_PlayerController::Client_CantUseItem_Implementation()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("You can't use this item!"));
}
void AM_PlayerController::Server_ApplyInventoryEffect_Implementation(bool bOpen)
{
	APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn());
	AM_PlayerState* PS = Cast<AM_PlayerState>(PC->GetPlayerState());
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();

	if (bOpen)
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Inventory.Open")));

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(InventoryOpenEffect, 1, Context);
		InventoryEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	
	}
	else
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Inventory.Open")));

		
	}
}




void AM_PlayerController::TraceForItem()
	{
		if (!IsLocalController()) return;

		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(GetPawn());
		if (!IsValid(PlayerChar)) return;

		FVector TraceStart = PlayerChar->GetFollowCamera()->GetComponentLocation();
		FVector TraceEnd = TraceStart + PlayerChar->GetFollowCamera()->GetForwardVector() * TraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(PlayerChar);

		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

		LastFocusedItem = FocusedItem;
		FocusedItem = HitResult.GetActor();

		if (FocusedItem == LastFocusedItem) return;

		if (LastFocusedItem.IsValid())
		{
			AM_Interactive_Item* OldItem = Cast<AM_Interactive_Item>(LastFocusedItem.Get());
			if (IsValid(OldItem))
				OldItem->OnUnfocused();
		}

		if (FocusedItem.IsValid())
		{
			AM_Interactive_Item* NewItem = Cast<AM_Interactive_Item>(FocusedItem.Get());
			if (IsValid(NewItem))
				NewItem->OnFocused();
		}

		
	}



