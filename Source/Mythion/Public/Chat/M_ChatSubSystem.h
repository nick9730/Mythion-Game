// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "M_ChatSubSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatMessageReceived, const FString&, Username, const FString&, Message, const FString&, Timestamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatDisconnected);

UCLASS()
class MYTHION_API UM_ChatSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ConnectToChat();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Username, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void DisconnectFromChat();

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatConnected OnChatConnected;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatDisconnected OnChatDisconnected;

private:
	TSharedPtr<IWebSocket> WebSocket;

	void HandleConnected();
	void HandleConnectionError(const FString& Error);
	void HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean);
	void HandleMessage(const FString& MessageString);
};