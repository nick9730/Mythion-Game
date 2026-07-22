// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "GAS/M_AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapons/WeaponDataASset.h"
#include "Weapons/WeaponBase.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create ASC
	//AbilitySystemComponent = CreateDefaultSubobject<UM_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	//AbilitySystemComponent->SetIsReplicated(true);
	//AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create AttributeSet
	//AttributeSet = CreateDefaultSubobject<UM_AttributeSet>(TEXT("AttributeSet"));

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(35.f, 96.f);

	// Rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
}
/*
UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
*/

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);


	/*
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (InitializeAttributesEffect)
		{
			InitializeAttributes(InitializeAttributesEffect);
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UM_AttributeSet::GetHealthAttribute())
			.AddUObject(this, &ABaseCharacter::OnHealthChanged);
	}
	*/
}

void ABaseCharacter::OnRep_PlayerState()
{
	
	/*

	Super::OnRep_PlayerState();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	*/
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseCharacter, bAlive);
	DOREPLIFETIME(ABaseCharacter, EquippedWeapon);
	DOREPLIFETIME(ABaseCharacter, EquippedArmor);

}

void ABaseCharacter::InitializeAttributes(TSubclassOf<UGameplayEffect> InitEffect) const
{
	/*
	if (!IsValid(InitEffect) || !IsValid(AbilitySystemComponent)) return;
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(InitEffect, 1.f, Context);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	*/
}

void ABaseCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f)
	{
		HandleDeath();
	}
}

void ABaseCharacter::HandleDeath()
{
	bAlive = false;

	if (HasAuthority())	
	{

		
		if (IsValid(EquippedWeapon))
		{
			
				EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				EquippedWeapon->Destroy();
			
		}
	}
	
}

void ABaseCharacter::SpawnWeapon()
{
	
	TArray<FWeaponDetails> WeaponDetails = WeaponData->Weapon;

	for (int i = 0; i < WeaponDetails.Num(); i++) {
		if (WeaponDetails[i].WeaponName == WeaponName)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();

			AWeaponBase* SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(
				WeaponDetails[i].WeaponClass,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);

			FName SocketName = WeaponDetails[i].SocketName;
			//FName("HandRightSocket")

			if (SpawnedWeapon)
			{
				FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
				SpawnedWeapon->AttachToComponent(GetMesh(), AttachRules, SocketName);
				SpawnedWeapon->WeaponDamage = WeaponDetails[i].TrueDamage;
				SpawnedWeapon->SetWeaponMesh(WeaponDetails[i].WeaponMesh);

			}
			if (SpawnedWeapon)
			{
				EquippedWeapon = SpawnedWeapon;
				EquippedArmor = SpawnedWeapon;
				
			}
		}
	}
}



























































