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
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level") 
    FGameplayTag Tag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
	TSubclassOf<UGameplayAbility> AbilityToGrant;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    UTexture2D* IconTag;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities Grant By Level")
    FGameplayTag CooldownTag;

};





USTRUCT(BlueprintType)
struct FCharacterClassData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Name tag")
    FGameplayTag ClassNameTag;

   // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class")
   // TSubclassOf<APlayerCharacter> CharacterClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icon of the player")
    UTexture2D* ClassIcon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle Animation For Selection")
    UAnimationAsset* ClassIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages")
    TMap<FGameplayTag, UAnimMontage*> SpellAnimationMap;



    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "All the abilities where player has")
    TMap<FGameplayTag , int32> GrantedAbilities;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Icons for Abilities")
    TMap<FGameplayTag, UTexture2D* > IconAbilities;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilitiesGrantByLevel")
	TArray<FGrantAbilitiesDataByLevel> AbilitiesGrantedByLevel;

     UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
     USkeletalMesh* CharacterMesh;

      UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character ABP")
      TSubclassOf<UAnimInstance> AnimBlueprintClass;
      
      UPROPERTY()
      UBlendSpace* BlendSpace;

  
};






UCLASS(BlueprintType)
class MYTHION_API UCharacterClasses : public UDataAsset
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Classes")
	TArray<struct FCharacterClassData> CharacterClasses;
};
