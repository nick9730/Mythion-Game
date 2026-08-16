// Fill out your copyright notice in the Description page of Project Settings.

#include "BackEnd/M_BackendSubsystem.h"
#include "HttpModule.h"
#include "M_PlayerController.h"

void UM_BackendSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Initialize called. ServerURL=%s"), *ServerURL);
}

void UM_BackendSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UM_BackendSubsystem::Login(const FString &Username, const FString &Password)
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Login() called. Username=%s, URL=%s"), *Username,
           *(ServerURL + TEXT("/api/auth/login")));

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    Request->SetURL(ServerURL + TEXT("/api/auth/login"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("password"), Password);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    UE_LOG(LogTemp, Warning, TEXT("[Backend] Login request body: %s"), *OutputString);

    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendSubsystem::OnLoginResponse);

    bool bRequestStarted = Request->ProcessRequest();
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Login ProcessRequest() returned: %d"), bRequestStarted);
}

void UM_BackendSubsystem::OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] OnLoginResponse called. bWasSuccessful=%d, ResponseValid=%d"),
           bWasSuccessful, Response.IsValid());

    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Login FAILED - connection failed. bWasSuccessful=%d"), bWasSuccessful);
        OnLoginComplete.Broadcast(false, TEXT("Connection failed"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Backend] Login Response Code: %d, Body: %s"), Response->GetResponseCode(),
           *Response->GetContentAsString());

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Login FAILED - could not parse JSON response"));
        OnLoginComplete.Broadcast(false, TEXT("Failed to parse response"));
        return;
    }

    if (Response->GetResponseCode() == 200)
    {
        AuthToken = JsonObject->GetStringField(TEXT("token"));
        LoggedInUsername = JsonObject->GetStringField(TEXT("username"));
        isNewPlayer = JsonObject->GetBoolField(TEXT("isNewPlayer"));
        bIsLoggedIn = true;

        UE_LOG(LogTemp, Warning, TEXT("[Backend] Login SUCCESS. Token=%s, Username=%s, isNewPlayer=%d"), *AuthToken,
               *LoggedInUsername, isNewPlayer);

        OnLoginComplete.Broadcast(true, TEXT("Login successful!"));
    }
    else
    {
        FString Message = JsonObject->GetStringField(TEXT("message"));
        UE_LOG(LogTemp, Error, TEXT("[Backend] Login FAILED - ResponseCode=%d, Message=%s"),
               Response->GetResponseCode(), *Message);
        OnLoginComplete.Broadcast(false, Message);
    }
}

void UM_BackendSubsystem::Register(const FString &Username, const FString &Email, const FString &Password,
                                   const FString &ConfirmPassword)
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Register() called. Username=%s, Email=%s"), *Username, *Email);

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

    UE_LOG(LogTemp, Warning, TEXT("[Backend] Register request body: %s"), *OutputString);

    Request->SetContentAsString(OutputString);
    Request->OnProcessRequestComplete().BindUObject(this, &UM_BackendSubsystem::OnRegisterResponse);

    bool bRequestStarted = Request->ProcessRequest();
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Register ProcessRequest() returned: %d"), bRequestStarted);
}

void UM_BackendSubsystem::OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] OnRegisterResponse called. bWasSuccessful=%d, ResponseValid=%d"),
           bWasSuccessful, Response.IsValid());

    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Register FAILED - connection failed"));
        OnRegisterComplete.Broadcast(false, TEXT("Connection failed"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Backend] Register Response Code: %d, Body: %s"), Response->GetResponseCode(),
           *Response->GetContentAsString());

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Register FAILED - could not parse JSON response"));
        OnRegisterComplete.Broadcast(false, TEXT("Failed to parse response"));
        return;
    }

    if (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201)
    {
        AuthToken = JsonObject->GetStringField(TEXT("token"));
        bIsLoggedIn = true;
        LoggedInUsername = JsonObject->GetStringField(TEXT("username"));

        UE_LOG(LogTemp, Warning, TEXT("[Backend] Register SUCCESS. Token=%s, Username=%s"), *AuthToken,
               *LoggedInUsername);

        OnRegisterComplete.Broadcast(true, TEXT("Registration successful! You can go to Character Selection Map."));
    }
    else
    {
        FString Message = JsonObject->GetStringField(TEXT("message"));
        UE_LOG(LogTemp, Error, TEXT("[Backend] Register FAILED - ResponseCode=%d, Message=%s"),
               Response->GetResponseCode(), *Message);
        OnRegisterComplete.Broadcast(false, Message);
    }
}

void UM_BackendSubsystem::Logout()
{
    UE_LOG(LogTemp, Warning, TEXT("[Backend] Logout() called."));

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
    UE_LOG(LogTemp, Warning, TEXT("[Backend] OnLogoutResponse called. bWasSuccessful=%d"), bWasSuccessful);

    if (bWasSuccessful && Response.IsValid() &&
        (Response->GetResponseCode() == 200 || Response->GetResponseCode() == 201))
    {
        AuthToken = TEXT("");
        LoggedInUsername = TEXT("");
        bIsLoggedIn = false;

        UE_LOG(LogTemp, Warning, TEXT("[Backend] Logout SUCCESS."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Backend] Logout FAILED."));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Logout failed"));
        }
    }
}