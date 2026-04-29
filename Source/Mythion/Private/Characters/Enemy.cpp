// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/Enemy.h"
#include "GAS/M_AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/Enemies/M_Enemy_Verticals.h"
#include "Components/CapsuleComponent.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "Weapons/WeaponBase.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/Enemy_Controller_Types.h"
#include "AIController.h"
#include "BrainComponent.h"



AEnemy::AEnemy()
{

	

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(200.f, 40.f));
	HealthBarComponent->SetVisibility(false);
	

		bReplicates = true;
		SetReplicateMovement(true);
}

UAbilitySystemComponent* AEnemy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEnemy::Tick(float DeltaTime)
{
	{
		Super::Tick(DeltaTime);

		if (!HealthBarComponent) return;

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) return;

		float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
		bool bShouldShow = Distance <= HealthBarVisibilityRadius;

	

	}
}

void AEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

void AEnemy::Multicast_PlayDeathAnimation_Implementation()
{
	if (IsValid(DeathMontage))
	{
		float Duration = PlayAnimMontage(DeathMontage);

		
		FTimerHandle DeathTimerHandle;
		GetWorldTimerManager().SetTimer(DeathTimerHandle, [this]()
			{
				if (IsValid(GetMesh()))
				{
					GetMesh()->bPauseAnims = true;
				}
			}, Duration, false);
	}


}



void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HealthBarWidgetClass && HealthBarComponent)
	{
		HealthBarComponent->SetWidgetClass(HealthBarWidgetClass);
		HealthBarComponent->InitWidget();

		if (UM_Enemy_Verticals* Widget = Cast<UM_Enemy_Verticals>(HealthBarComponent->GetWidget()))
		{
			if (IsValid(AbilitySystemComponent))
			{
				int32 Level = FMath::FloorToInt(
					AbilitySystemComponent->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute()));
				Widget->InitializeWidget(AbilitySystemComponent, EnemyDisplayName, Level);
			}
		}

		if (HasAuthority() && IsValid(AbilitySystemComponent))
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(AbilityClass, 1)
			);
		}

	}


	if (HasAuthority())
	{
		if (IsValid(AbilitySystemComponent))
		{
			AbilitySystemComponent->InitAbilityActorInfo(this, this);

			if (IsValid(InitializeAttributesEffect))
			{
				InitializeAttributes(InitializeAttributesEffect);
			}
		}
	}
	if (HealthBarWidgetClass && HealthBarComponent)
	{
		HealthBarComponent->SetWidgetClass(HealthBarWidgetClass);
		HealthBarComponent->InitWidget();
		HealthBarComponent->SetVisibility(true);

		if (UM_Enemy_Verticals* Widget = Cast<UM_Enemy_Verticals>(HealthBarComponent->GetWidget()))
		{
			Widget->SetOwningEnemy(this);
		}
	}


	if (HasAuthority()) {
		if (WeaponData) { SpawnWeapon(); }
	}

}

void AEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);



}


void AEnemy::HandleDeath()
{

	AM_Enemy_Controller* AIController = Cast<AM_Enemy_Controller>(GetController());
	if (IsValid(AIController) && IsValid(AIController->GetBlackboardComponent()))
	{
		AIController->GetBlackboardComponent()->SetValueAsEnum(
			FName("State"), (uint8)E_AIStates::Dead);
		AIController->StopMovement();
		AIController->BrainComponent->StopLogic(TEXT("Dead"));
	}
	Super::HandleDeath();

	
	if (HasAuthority())
	{
		Multicast_HideWeapon(); 
		Multicast_PlayDeathAnimation();

	
		if (IsValid(EquippedWeapon))
		{
			EquippedWeapon->SetLifeSpan(0.1f);
			EquippedWeapon = nullptr;
		}
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(3.f);
}

void AEnemy::Multicast_HideWeapon_Implementation()
{
	if (IsValid(EquippedWeapon))
		EquippedWeapon->SetActorHiddenInGame(true);
}
