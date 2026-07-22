// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterSelectionWidget.h"
#include "Components/Button.h"
#include "Widgets/M_SelectionStats.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "DataAsset/CharacterClasses.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "BackEnd/M_BackendSubsystem.h"
#include "Dom/JsonObject.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Characters/PlayerCharacter.h"
#include "M_PlayerController.h"
#include "Widgets/M_AbilitiesOfClasses.h"
#include "Components/VerticalBox.h"

void UCharacterSelectionWidget::OnConfirmButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) { UE_LOG(LogTemp, Error, TEXT("PC null")); return; }

	ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
	if (!IsValid(Character)) { UE_LOG(LogTemp, Error, TEXT("Character null")); return; }

	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character);
	if (!ASCInterface) { UE_LOG(LogTemp, Error, TEXT("ASCInterface null")); return; }

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!IsValid(ASC)) { UE_LOG(LogTemp, Error, TEXT("ASC null")); return; }

	FCharacterClassData& Data = CharacterChoiceData->CharacterClasses[CurrentCharacterIndex];


	TSharedPtr<FJsonObject> SendToBackendObject = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();


	SendToBackendObject->SetStringField(TEXT("className"), Data.ClassNameTag.GetTagName().ToString());
	SendToBackendObject->SetNumberField(TEXT("level"), 1);
	SendToBackendObject->SetNumberField(TEXT("xp"), 0);
	SendToBackendObject->SetNumberField(TEXT("health"), Health);
	SendToBackendObject->SetNumberField(TEXT("mana"), Mana);
	SendToBackendObject->SetNumberField(TEXT("armor"), Armor);
	SendToBackendObject->SetNumberField(TEXT("coins"), 0);
	SendToBackendObject->SetNumberField(TEXT("magicResist"),MagicResistance);

	TSharedPtr<FJsonObject> LocationObj = MakeShared<FJsonObject>();
	LocationObj->SetNumberField(TEXT("x"), 200);
	LocationObj->SetNumberField(TEXT("y"), 200);
	LocationObj->SetNumberField(TEXT("z"), 200);
	SendToBackendObject->SetObjectField(TEXT("lastLocation"), LocationObj);


	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(SendToBackendObject.ToSharedRef(), Writer);

	

	UM_BackendSubsystem* Backend = PC->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
	if (!Backend || Backend->AuthToken.IsEmpty()) return;

	if (!Backend) { UE_LOG(LogTemp, Error, TEXT("Backend null")); return; }
	if (Backend->AuthToken.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("AuthToken empty")); return; }

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Sending stats - Body: %s"), *Body));

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Backend->ServerURL + TEXT("/api/player/stats"));
	Request->SetVerb(TEXT("PUT"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UCharacterSelectionWidget::OnConfirmComplete);
	Request->ProcessRequest();

	

}

void UCharacterSelectionWidget::OnNextButtonClicked()
{
	for (int32 i = 1; i <= CharacterChoiceData->CharacterClasses.Num(); i++)
	{
		int32 NextIndex = (CurrentCharacterIndex + i) % CharacterChoiceData->CharacterClasses.Num();
		if (CharacterChoiceData->CharacterClasses[NextIndex].CharacterMesh != nullptr)
		{
			CurrentCharacterIndex = NextIndex;

			
			APlayerController* PC = GetOwningPlayer();
			if (!IsValid(PC)) break;

			ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
			if (!IsValid(Character)) break;

			FCharacterClassData& Data = CharacterChoiceData->CharacterClasses[CurrentCharacterIndex];
			Character->GetMesh()->SetSkeletalMesh(Data.CharacterMesh);
			Character->GetMesh()->SetAnimInstanceClass(Data.AnimBlueprintClass);
			UpdateStatsDisplay(CharacterChoiceData->CharacterClasses[CurrentCharacterIndex]);
			CreateAbilitiesInfos(Data);


			break;
		}
	}

}

void UCharacterSelectionWidget::OnPreviousButtonClicked()
{
	for (int32 i = 1; i <= CharacterChoiceData->CharacterClasses.Num(); i++)
	{
		int32 PrevIndex = (CurrentCharacterIndex - i + CharacterChoiceData->CharacterClasses.Num()) % CharacterChoiceData->CharacterClasses.Num();
		if (CharacterChoiceData->CharacterClasses[PrevIndex].CharacterMesh != nullptr)
		{
			APlayerController* PC = GetOwningPlayer();
			if (!IsValid(PC)) break;

			ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
			if (!IsValid(Character)) break;

			CurrentCharacterIndex = PrevIndex; 

			FCharacterClassData& Data = CharacterChoiceData->CharacterClasses[CurrentCharacterIndex];
			Character->GetMesh()->SetSkeletalMesh(Data.CharacterMesh);
			Character->GetMesh()->SetAnimInstanceClass(Data.AnimBlueprintClass);
			UpdateStatsDisplay(CharacterChoiceData->CharacterClasses[CurrentCharacterIndex]);
			CreateAbilitiesInfos(Data);
			break;
		}
	}
}

void UCharacterSelectionWidget::NativeConstruct()
{
		Super::NativeConstruct();

		 APlayerController* PC = GetOwningPlayer();
		if (NextButton)
			NextButton->OnClicked.AddDynamic(this, &UCharacterSelectionWidget::OnNextButtonClicked);

		if (PreviousButton)
			PreviousButton->OnClicked.AddDynamic(this, &UCharacterSelectionWidget::OnPreviousButtonClicked);

		if (ConfirmButton)
			ConfirmButton->OnClicked.AddDynamic(this, &UCharacterSelectionWidget::OnConfirmButtonClicked);

		if (CharacterChoiceData && CharacterChoiceData->CharacterClasses.IsValidIndex(0))
		{
			ACharacter* Character = PC ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
			if (Character)
			{
				FCharacterClassData& Data = CharacterChoiceData->CharacterClasses[0];
				Character->GetMesh()->SetSkeletalMesh(Data.CharacterMesh);
				Character->GetMesh()->SetAnimInstanceClass(Data.AnimBlueprintClass);
				UpdateStatsDisplay(Data);
				CreateAbilitiesInfos(Data);
			}
		}
	
		if (IsValid(PC)) {
			FInputModeUIOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
}

void UCharacterSelectionWidget::UpdateStatsDisplay(FCharacterClassData  SelectedData)
{
	
	Mana = SelectedData.PreviewMana;
	Armor = SelectedData.PreviewArmor;
	Health = SelectedData.PreviewHealth;
	MagicResistance = SelectedData.PreviewMagicResistance;

	if (!IsValid(StatsWidget)) return;

	if (StatsWidget->NameText)
		StatsWidget->NameText->SetText(FText::FromString(SelectedData.NameClassPlayer.ToString()));

	if (StatsWidget->ArmorValue)
		StatsWidget->ArmorValue->SetText(FText::AsNumber(FMath::RoundToInt(SelectedData.PreviewArmor)));

	if (StatsWidget->ManaValue)
		StatsWidget->ManaValue->SetText(FText::AsNumber(FMath::RoundToInt(SelectedData.PreviewMana)));

	if (StatsWidget->HealthValue)
		StatsWidget->HealthValue->SetText(FText::AsNumber(FMath::RoundToInt(SelectedData.PreviewHealth)));

	if (StatsWidget->MagicResistanceValue)
		StatsWidget->MagicResistanceValue->SetText(FText::AsNumber(FMath::RoundToInt(SelectedData.PreviewMagicResistance)));

	if (Description)
	{
		Description->SetText(SelectedData.Description);
  }

	}


void UCharacterSelectionWidget::OnConfirmComplete(TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request, TSharedPtr<IHttpResponse, ESPMode::ThreadSafe> Response, bool bWasSuccessful)
{



	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) { UE_LOG(LogTemp, Error, TEXT("PC null")); return; }
	UM_BackendSubsystem* Backend = PC->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
	if (!Backend || Backend->AuthToken.IsEmpty()) return;

	if (!bWasSuccessful || !Response.IsValid())
	{
		return;
	}

	if (Response->GetResponseCode() >= 200 && Response->GetResponseCode() < 300)
	{
	
		APlayerController* PCharacter = GetOwningPlayer();
		AM_PlayerController* MPC = Cast<AM_PlayerController>(PCharacter);
	
		Backend->isNewPlayer = false;
		TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
		Json->SetBoolField(TEXT("isNewPlayer"), false);
		FString Body;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

		TSharedRef<IHttpRequest> Req = FHttpModule::Get().CreateRequest();
		Req->SetURL(Backend->ServerURL + TEXT("/api/auth/newplayer"));
		Req->SetVerb(TEXT("PUT"));
		Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Backend->AuthToken));
		Req->SetContentAsString(Body);
		Req->ProcessRequest();

		PC->ClientTravel(TEXT("10.227.124.168"), ETravelType::TRAVEL_Absolute);

	}
	else
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(Reader, JsonObject);
		FString Message = JsonObject.IsValid() ? JsonObject->GetStringField(TEXT("message")) : TEXT("Unknown error");
		
	}
}



void UCharacterSelectionWidget::CreateAbilitiesInfos(FCharacterClassData SelectedClassData) {

	if (!AbilitiesContainer || !AbilitiesInfos) return;

	AbilitiesContainer->ClearChildren();
		
	for (const FGrantAbilitiesDataByLevel& AbilityEntry : SelectedClassData.AbilitiesGrantedByLevel)
	{
		if (!IsValid(AbilityEntry.AbilityToGrant)) continue;
		if (AbilityEntry.bIsPassive) continue;
		UM_AbilitiesOfClasses* AbilityWidget = CreateWidget<UM_AbilitiesOfClasses>(this, AbilitiesInfos);
		if (!IsValid(AbilityWidget)) continue;

		

		AbilityWidget->InitAbilityBox(AbilityEntry.NameAbility, AbilityEntry.IconTag, AbilityEntry.RequiredLevel);

		AbilitiesContainer->AddChildToVerticalBox(AbilityWidget);
	}
	

}