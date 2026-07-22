// Fill out your copyright notice in the Description page of Project Settings.


#include "BackEnd/M_BackendSubsystem.h"
#include "HttpModule.h"
#include "M_PlayerController.h"

void UM_BackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UM_BackendSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UM_BackendSubsystem::Login(const FString& Username, const FString& Password)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ServerURL + TEXT("/api/auth/login"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    // Build JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("password"), Password);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendSubsystem::OnLoginResponse);
    Request->ProcessRequest();
}


void UM_BackendSubsystem::OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        OnLoginComplete.Broadcast(false, TEXT("Connection failed"));
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        OnLoginComplete.Broadcast(false, TEXT("Failed to parse response"));
        return;
    }

    if (Response->GetResponseCode() == 200)
    {
        AuthToken = JsonObject->GetStringField(TEXT("token"));
        LoggedInUsername = JsonObject->GetStringField(TEXT("username"));
      isNewPlayer = JsonObject->GetBoolField(TEXT("isNewPlayer"));
        bIsLoggedIn = true;
        OnLoginComplete.Broadcast(true, TEXT("Login successful!"));

		
    }
    else
    {
        FString Message = JsonObject->GetStringField(TEXT("message"));
        OnLoginComplete.Broadcast(false, Message);
    }
   
}

void UM_BackendSubsystem::Register(const FString& Username, const FString& Email, const FString& Password, const FString& ConfirmPassword)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ServerURL + TEXT("/api/auth/register"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("email"), Email);
    JsonObject->SetStringField(TEXT("password"), Password);
    JsonObject->SetStringField(TEXT("confirmPassword"), ConfirmPassword);


    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendSubsystem::OnRegisterResponse);
    Request->ProcessRequest();



}

void UM_BackendSubsystem::OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{

    


    if (!bWasSuccessful || !Response.IsValid())
    {
        OnRegisterComplete.Broadcast(false, TEXT("Connection failed"));
        return;
    }
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        OnRegisterComplete.Broadcast(false, TEXT("Failed to parse response"));
        return;
    }
    if (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201)
    {
        AuthToken = JsonObject->GetStringField(TEXT("token"));
        bIsLoggedIn = true;
        LoggedInUsername = JsonObject->GetStringField(TEXT("username"));
        OnRegisterComplete.Broadcast(true, TEXT("Registration successful! You can  go to Character Selection Map."));
    }
    else
    {
      ;
        FString Message = JsonObject->GetStringField(TEXT("message"));
        OnRegisterComplete.Broadcast(false, Message);
       

	}
}


void UM_BackendSubsystem::Logout()
{
    FString URL = ServerURL + TEXT("/api/auth/logout");

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));

    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendSubsystem::OnLogoutResponse);
    Request->ProcessRequest();
}


void UM_BackendSubsystem::OnLogoutResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid() && (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201))
    {
        AuthToken = TEXT("");
        LoggedInUsername = TEXT("");
        bIsLoggedIn = false;

    
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Logout failed"));
        }
    }
}

