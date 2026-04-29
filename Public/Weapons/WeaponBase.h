// Fill out your copyright notice in the Description page of Project Settings.

#pragma once





#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "WeaponBase.generated.h"

class UStaticMeshComponent;
class UGameplayEffect;

UCLASS()
class MYTHION_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool SetHitScanEnabled(bool bEnabled);
	void SetWeaponMesh(UStaticMesh* NewMesh);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float WeaponDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName SocketStart = FName("TraceStart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName SocketEnd = FName("TraceEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FGameplayTag DamageTypeTag;

	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(AActor* HitActor);

protected:
	virtual void BeginPlay() override;

private:
	void PerformWeaponTrace();
	void ApplyDamage(AActor* HitActor);

	UFUNCTION()
	void OnRep_WeaponMesh();

	UPROPERTY(ReplicatedUsing = OnRep_WeaponMesh)
	TObjectPtr<UStaticMesh> WeaponMeshToUse;

	TArray<AActor*> AlreadyHitActors;
	bool bIsScanning = false;
};



