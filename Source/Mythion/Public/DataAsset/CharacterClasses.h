// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h" 
#include "CharacterClasses.generated.h"



class APlayerCharacter;
class UTexture2D;
class UAnimInstance;
class USkeletalMesh;
class UAnimMontage;
struct FGameplayTag;
class UBlendSpace;
class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;




USTRUCT(BlueprintType)
struct FGrantAbilitiesDataByLevel
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
	int32 RequiredLevel =1 ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level") 
    FGameplayTag Tag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
	TSubclassOf<UGameplayAbility> AbilityToGrant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    UTexture2D* IconTag = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    int32 InputID;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    FName NameAbility;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    FString   KeyOfButtonAbility;



    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    FGameplayTag CooldownTag;


    UPROPERTY(EditAnywhere)
    bool bIsPassive = false;

    UPROPERTY(EditAnywhere)
    bool bIsCommonToAllClasses=false;
};





USTRUCT(BlueprintType)
struct FCharacterClassData
{
    GENERATED_BODY()

public:

    //description of the player and hero 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Name tag")
    FGameplayTag ClassNameTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Name of the class player")
    FName NameClassPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon of the player")
    UTexture2D* ClassIcon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    USkeletalMesh* CharacterMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character ABP")
    TSubclassOf<UAnimInstance> AnimBlueprintClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Montage")
    UAnimMontage* DeathMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InitialLocationOfCharacter")
	FVector InitialLocationOfCharacter = FVector::ZeroVector;

 //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle Animation For Selection")
    UAnimationAsset* ClassIdleAnimation=nullptr;


	//Abilities and Granted Abilities related
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitiesGrantByLevel")
    TArray<FGrantAbilitiesDataByLevel> AbilitiesGrantedByLevel;


	//Effects  related
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gas Effects")
    TSubclassOf<UGameplayEffect> ManaRegenPerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gas Effects")
    TSubclassOf<UGameplayEffect> HealthRegenPerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gas Effects")
    TSubclassOf<UGameplayEffect> UpdateStatsPerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gas Effects")
    TSubclassOf<UGameplayEffect> RegisterStatsPerType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gas Effects")
    TSubclassOf<UGameplayEffect> EnergyRegenPerType;


    UPROPERTY(EditAnywhere, Category = "Preview Stats")
    float PreviewHealth = 100.f;

    UPROPERTY(EditAnywhere, Category = "Preview Stats")
    float PreviewMana = 100.f;

    UPROPERTY(EditAnywhere, Category = "Preview Stats")
    float PreviewArmor = 0.f;

    UPROPERTY(EditAnywhere, Category = "Preview Stats")
    float PreviewMagicResistance = 0.f;

    UPROPERTY(EditAnywhere, Category = "Preview Stats")
    FText Description;


};






UCLASS(BlueprintType)
class MYTHION_API UCharacterClasses : public UDataAsset
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Classes")
	TArray<struct FCharacterClassData> CharacterClasses;



    TArray<FGrantAbilitiesDataByLevel> CommonAbilities;
};


//UPROPERTY(EditAnywhere, BlueprintReadOnly)
//UBlendSpace* BlendSpace= nullptr;

   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class")
   // TSubclassOf<APlayerCharacter> CharacterClass;



   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle Animation For Selection")
  //  UAnimationAsset* ClassIdleAnimation=nullptr;

   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
  //  TMap<FGameplayTag, UAnimMontage*> SpellAnimationMap;



   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All the abilities where player has")
   // TMap<FGameplayTag , int32> GrantedAbilities;
   // 
   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons for Abilities")
   // TMap<FGameplayTag, UTexture2D* > IconAbilities;