// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "M_BackendSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthComplete, bool, bSuccess, const FString&, Message);

/**
 * 
 */
UCLASS()
class MYTHION_API UM_BackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadWrite, Category = "Mythion | Backend")
    FString ServerURL= TEXT("http://10.86.209.168:3000");
     
    // Auth data
    UPROPERTY(BlueprintReadOnly, Category = "Mythion | Backend")
    FString AuthToken;

    UPROPERTY(BlueprintReadOnly, Category = "Mythion | Backend")
    FString LoggedInUsername;

    UPROPERTY(BlueprintReadOnly, Category = "Mythion | Backend")
    bool bIsLoggedIn = false;

    UPROPERTY(BlueprintReadOnly, Category = "Mythion | Backend")
	bool isNewPlayer = false;

    // Delegates - Blueprint can bind to these
    UPROPERTY(BlueprintAssignable, Category = "Mythion | Backend")
    FOnAuthComplete OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "Mythion | Backend")
    FOnAuthComplete OnRegisterComplete;

    UFUNCTION(BlueprintCallable, Category = "Mythion | Backend")
    void Login(const FString& Username, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "Mythion | Backend")
    void Register(const FString& Username, const FString& Email, const FString& Password,const FString& ConfirmPassword);

    UFUNCTION(BlueprintCallable, Category = "Mythion | Backend")
    void Logout();
	





private:
    void OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnLogoutResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
