// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat/M_ChatSubSystem.h"

#include "WebSocketsModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UM_ChatSubSystem::Deinitialize()
{
	DisconnectFromChat();
	Super::Deinitialize();
}

void UM_ChatSubSystem::ConnectToChat()
{
	UE_LOG(LogTemp, Warning, TEXT("ConnectToChat called!"));

	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("Already connected, skipping"));
		return;
	}
	// Ίδιο IP/port με το BackendSubsystem HTTP, απλά ws:// αντί για http://
	const FString ServerURL = TEXT("ws://10.41.164.168:3000/socket.io/?EIO=4&transport=websocket");

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL);

	WebSocket->OnConnected().AddUObject(this, &UM_ChatSubSystem::HandleConnected);
	WebSocket->OnConnectionError().AddUObject(this, &UM_ChatSubSystem::HandleConnectionError);
	WebSocket->OnClosed().AddUObject(this, &UM_ChatSubSystem::HandleClosed);
	WebSocket->OnMessage().AddUObject(this, &UM_ChatSubSystem::HandleMessage);

	WebSocket->Connect();
}

void UM_ChatSubSystem::DisconnectFromChat()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Close();
	}
	WebSocket.Reset();
}

void UM_ChatSubSystem::SendChatMessage(const FString& Username, const FString& Message)
{
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField("username", Username);
	JsonPayload->SetStringField("message", Message);

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	// socket.io v4 text frame format: 42["event_name", payload]
	const FString Frame = FString::Printf(TEXT("42[\"chat_message\",%s]"), *PayloadString);
	WebSocket->Send(Frame);
}

void UM_ChatSubSystem::HandleConnected()
{
	OnChatConnected.Broadcast();
}

void UM_ChatSubSystem::HandleConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Warning, TEXT("Chat WebSocket connection error: %s"), *Error);
}

void UM_ChatSubSystem::HandleClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	OnChatDisconnected.Broadcast();
}

void UM_ChatSubSystem::HandleMessage(const FString& MessageString)
{
	UE_LOG(LogTemp, Warning, TEXT("Chat RAW message: %s"), *MessageString);
	if (MessageString == TEXT("2"))
	{
		if (WebSocket.IsValid() && WebSocket->IsConnected())
		{
			WebSocket->Send(TEXT("3"));
		}
		return;
	}

	if (MessageString.StartsWith(TEXT("0{")))
	{
		if (WebSocket.IsValid() && WebSocket->IsConnected())
		{
			WebSocket->Send(TEXT("40"));
		}
		return;
	}

	if (MessageString.StartsWith(TEXT("42")))
	{
		const FString JsonArrayString = MessageString.RightChop(2); // αφαιρούμε το "42"

		TSharedPtr<FJsonValue> ParsedValue;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonArrayString);

		if (FJsonSerializer::Deserialize(Reader, ParsedValue) && ParsedValue.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* JsonArray;
			if (ParsedValue->TryGetArray(JsonArray) && JsonArray->Num() >= 2)
			{
				const FString EventName = (*JsonArray)[0]->AsString();

				if (EventName == TEXT("chat_message"))
				{
					const TSharedPtr<FJsonObject> PayloadObject = (*JsonArray)[1]->AsObject();
					if (PayloadObject.IsValid())
					{
						const FString Username = PayloadObject->GetStringField(TEXT("username"));
						const FString Message = PayloadObject->GetStringField(TEXT("message"));
						const FString Timestamp = PayloadObject->GetStringField(TEXT("timestamp"));

						OnChatMessageReceived.Broadcast(Username, Message, Timestamp);
						UE_LOG(LogTemp, Warning, TEXT("Parsed chat -> %s: %s"), *Username, *Message);

					}
				}
			}
		}
		return;
	}

}