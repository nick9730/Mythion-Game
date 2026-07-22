// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponDataASset.generated.h"

class UGameplayEffect;
class UStaticMesh;
struct FGameplayTag;


USTRUCT()
struct FWeaponDetails
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere,  Category = "Visuals")
    FName WeaponName;

    UPROPERTY(EditAnywhere, Category = "Visuals")
    FName SocketName;

    UPROPERTY(EditAnywhere,  Category = "Visuals")
    UStaticMesh* WeaponMesh;

    UPROPERTY(EditAnywhere,  Category = "Stats")
    float WeaponDamage;

    UPROPERTY(EditAnywhere,  Category = "Stats")
    float TrueDamage;

    UPROPERTY(EditAnywhere,  Category = "Stats")
	float MagicDamage;

	UPROPERTY(EditAnywhere, Category = "Stats")
	float AttackSpeed;


    UPROPERTY(EditAnywhere, Category = "Stats")
    float Armor;


    UPROPERTY(EditAnywhere, Category = "Stats")
    FGameplayTag DamageTypeTag; 


    UPROPERTY(EditAnywhere, Category = "GAS")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditAnywhere, Category = "Weapon BP")
    TSubclassOf<class AWeaponBase> WeaponClass;


};


UCLASS()
class MYTHION_API UWeaponDataASset : public UPrimaryDataAsset
{
	GENERATED_BODY()


public:
    UPROPERTY(EditAnywhere,  Category = "GAS")
	TArray<FWeaponDetails> Weapon;
	
};
