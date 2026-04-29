// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "DataAsset/CharacterClasses.h"
#include "Characters/BaseCharacter.h"
#include "GameplayEffectTypes.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "M_PlayerController.h"
#include "Components/SceneCaptureComponent2D.h"



APlayerCharacter::APlayerCharacter()
{
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArmComp->SetupAttachment(GetRootComponent());
	SpringArmComp->TargetArmLength = 600.f;
	SpringArmComp->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	


}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
	if (WeaponData)	{SpawnWeapon();}
	}

	if (IsLocallyControlled())
	{
		PreviewCapture = NewObject<USceneCaptureComponent2D>(this, TEXT("PreviewCapture"));
		PreviewCapture->SetupAttachment(GetMesh());
		PreviewCapture->SetRelativeLocation(FVector(-200.f, 0.f, 100.f));
		PreviewCapture->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
		PreviewCapture->bCaptureEveryFrame = true;
		PreviewCapture->RegisterComponent();

		UTextureRenderTarget2D* NewRT = NewObject<UTextureRenderTarget2D>(this);
		NewRT->InitAutoFormat(512, 512);
		NewRT->UpdateResourceImmediate(true);
		PreviewCapture->TextureTarget = NewRT;
		PreviewRenderTarget = NewRT;

		OnPreviewReady.Broadcast(PreviewRenderTarget);
	}
	Tags.Add(FName("Player"));




}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(PS)) return;
	PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);
	ApplyCharacterClassData(CharacterClassData, PlayerClassTag);
	SetupAttributes();
	Tags.Add(FName("Player"));
	



}


// State,Attribute,ASC

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(PS)) return;


	PS->GetAttributeSet()->OnLevelUp.AddDynamic(this, &APlayerCharacter::LevelUp);
	

	PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

	PS->GetAbilitySystemComponent()->ForceReplication();

	PS->GetAttributeSet()->OnLevelUp.AddDynamic(this, &APlayerCharacter::LevelUp);

	AM_PlayerController* PC = Cast<AM_PlayerController>(GetController());
	if (IsValid(PC) && IsLocallyControlled())
	{
		PC->TryInitInventory();
	}

	OnASCInitialized.Broadcast(PS->GetAbilitySystemComponent(), Cast<UAttributeSet>(PS->GetAttributeSet()));

}


UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	UE_LOG(LogTemp, Warning, TEXT("GetASC called - PS valid: %s"), IsValid(PS) ? TEXT("YES") : TEXT("NO"));
	if (!IsValid(PS)) return nullptr;

	return PS->GetAbilitySystemComponent();
}

UAttributeSet* APlayerCharacter::GetAttributeSet() const
{
	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(PS)) return nullptr;
	return PS->GetAttributeSet();
}

//Replicate
void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


//SetupInitial Attributes and mesh
void APlayerCharacter::ApplyCharacterClassData(TSoftObjectPtr<UCharacterClasses> ClassData, const FGameplayTag TagMatches)
{
	USkeletalMeshComponent* MeshComp = GetMesh();

	if (!ClassData)	return;
	if (!MeshComp)  return;

	for (const FCharacterClassData& Choice : ClassData->CharacterClasses)
	{
		UE_LOG(LogTemp, Error, TEXT("COMPARING: AssetTag(%s) WITH SearchTag(%s)"), *Choice.ClassNameTag.ToString(), *TagMatches.ToString());
		if (Choice.ClassNameTag.MatchesTagExact(TagMatches))
		{
			if (Choice.CharacterMesh) {
				MeshComp->SetSkeletalMesh(Choice.CharacterMesh);
			}
			else {
			}

			MeshComp->SetAnimInstanceClass(Choice.AnimBlueprintClass);

			return;
		}
	}
}

void APlayerCharacter::SetupAttributes()
{
	AM_PlayerState* PlayState = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(PlayState)) return;
	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	UAbilitySystemComponent* ASC = PlayState->GetAbilitySystemComponent();

	for (const FCharacterClassData& ClassData : CharacterClassData->CharacterClasses)
	{
		if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag)) continue;

		for (const FGrantAbilitiesDataByLevel& AbilityEntry : ClassData.AbilitiesGrantedByLevel)
		{
			if (AbilityEntry.RequiredLevel == 1)
			{
				GetAbilitySystemComponent()->GiveAbility(
					FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1)
				);
				OnAbilityGranted.Broadcast(AbilityEntry);
			}
		}
	}

	if (!IsValid(PS)) return;

	PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);


	if (InitializeAttributesEffect)
	{
		InitializeAttributes(InitializeAttributesEffect);
	}



	ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetLevelAttribute())
		.AddUObject(this, &APlayerCharacter::OnLevelAttributeChanged);

	

	OnASCInitialized.Broadcast(
		PS->GetAbilitySystemComponent(),
		Cast<UAttributeSet>(PS->GetAttributeSet())
	);

	


}

void APlayerCharacter::OnLevelAttributeChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("Level changed! New Level: %f"), Data.NewValue);
	LevelUp(Data.NewValue);
}




// Levels and Abilities
void APlayerCharacter::LevelUp(float NewLevel)
{
	UE_LOG(LogTemp, Warning, TEXT("OnLevelUp called %f"), NewLevel);
	if (IsValid(CharacterClassData))
	{
		for (const FCharacterClassData& ClassData : CharacterClassData->CharacterClasses)
		{
			if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag)) continue;

			for (const FGrantAbilitiesDataByLevel& AbilityEntry : ClassData.AbilitiesGrantedByLevel)
			{
				if (AbilityEntry.RequiredLevel == FMath::FloorToInt(NewLevel))
				{
					GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1)
					);

					Multicast_OnAbilityGranted(AbilityEntry);
					OnAbilityInitialized.Broadcast(AbilityEntry);
					UE_LOG(LogTemp, Warning, TEXT("OnAbilityGranted bound: %s"), OnAbilityGranted.IsBound() ? TEXT("YES") : TEXT("NO"));
					OnLevelChanged.Broadcast(NewLevel);
				}
			}
		}


	}
}

void APlayerCharacter::GrantAbilities()
{
	if (!IsValid(CharacterClassData)) return;
	AM_PlayerState* PlayState = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(PlayState)) return;
	AM_PlayerState* PS = Cast<AM_PlayerState>(GetPlayerState());
	UAbilitySystemComponent* ASC = PlayState->GetAbilitySystemComponent();


	for (const FCharacterClassData& ClassData : CharacterClassData->CharacterClasses)
	{
		if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag)) continue;

		for (const FGrantAbilitiesDataByLevel& AbilityEntry : ClassData.AbilitiesGrantedByLevel)
		{
			if (IsValid(AbilityEntry.AbilityToGrant) && AbilityEntry.RequiredLevel <= FMath::FloorToInt(GetAbilitySystemComponent()->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute())))
			{
				ASC->GiveAbility(FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1));
			}
		}


	}

}

void APlayerCharacter::Multicast_OnAbilityGranted_Implementation(FGrantAbilitiesDataByLevel AbilityEntry)
{

	if (!HasAuthority())
	{
		OnAbilityGranted.Broadcast(AbilityEntry);
	}
}



/*


#include "Characters/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/CharacterClasses.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponDataASset.h"
#include "Attributes/M_AttributeSet.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "Characters/BaseCharacter.h"
	


APlayerCharacter::APlayerCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	SpringArmComp->SetupAttachment(GetRootComponent());
	SpringArmComp->TargetArmLength = 600.f;
	SpringArmComp->bUsePawnControlRotation = true;
    Camera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	Camera->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	AttributesCharacter = CreateDefaultSubobject<UM_AttributeSet>("AttributSet");


}

void APlayerCharacter::SpawnWeapon()
{
	Super::SpawnWeapon();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponData)
	{
		SpawnWeapon();
	}
	
	
}
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* APlayerCharacter::GetAbilitySystemComponent() const
{
	AM_PlayerState* MPlayerState = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(MPlayerState)) return nullptr;
	
	return MPlayerState->GetAbilitySystemComponent();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!IsValid(GetAbilitySystemComponent()) || !HasAuthority()) return;
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);
	InitializeAttributes(InitializeAttributesEffect);


	AttributesCharacter = Cast<UM_AttributeSet>(GetAttributeSet());
	if (!IsValid(AttributesCharacter)) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(AttributesCharacter->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);


}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!IsValid(GetAbilitySystemComponent())) return;
	GetAbilitySystemComponent()->InitAbilityActorInfo(GetPlayerState(), this);


	UE_LOG(LogTemp, Warning, TEXT("OnRep_PlayerState broadcasting OnASCInitialized"));

	OnASCInitialized.Broadcast(GetAbilitySystemComponent(), GetAttributeSet());


	UM_AttributeSet* AttributeSet = Cast<UM_AttributeSet>(GetAttributeSet());

	if (!IsValid(AttributeSet)) return;
	GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);

}

UAttributeSet* APlayerCharacter::GetAttributeSet() const
{
	AM_PlayerState* MPlayerState = Cast<AM_PlayerState>(GetPlayerState());
	if (!IsValid(MPlayerState)) return nullptr;
	return MPlayerState->GetAttributeSet();
}



void APlayerCharacter::ApplyCharacterClassData(TSoftObjectPtr<UCharacterClasses> ClassData,const FGameplayTag TagMatches)
{
	USkeletalMeshComponent* MeshComp = GetMesh();

	if (!ClassData)	return;
	if (!MeshComp)  return;

	for (const FCharacterClassData& Choice : ClassData->CharacterClasses)
     {
		UE_LOG(LogTemp, Error, TEXT("COMPARING: AssetTag(%s) WITH SearchTag(%s)"),*Choice.ClassNameTag.ToString(), *TagMatches.ToString());
	  if(Choice.ClassNameTag.MatchesTagExact(TagMatches))
	  {
		  if (Choice.CharacterMesh) {
			  MeshComp->SetSkeletalMesh(Choice.CharacterMesh);
		  }
		  else {
		  }
		
		  MeshComp->SetAnimInstanceClass(Choice.AnimBlueprintClass);

		  return;
	  }
  }
}


/*
FVector APlayerCharacter::GetTranslationWarpTarget()
{

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());


	if (PlayerCharacter == nullptr) return FVector();
	const FVector CombatTargetLocation = PlayerCharacter->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}

FVector APlayerCharacter::GetRotationWarpTarget()
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOwner());

	if (PlayerCharacter)
	{
		return PlayerCharacter->GetActorLocation();
	}
	return FVector();
}
*/
