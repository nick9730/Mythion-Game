// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat/M_ChatWidget.h"
#include "Chat/M_ChatSubSystem.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "BackEnd/M_BackendSubsystem.h"

void UM_ChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UM_ChatSubSystem* ChatSub = GetChatSubsystem())
	{
		ChatSub->OnChatMessageReceived.AddDynamic(this, &UM_ChatWidget::HandleMessageReceived);
	}

	if (SendButton)
	{
		SendButton->OnClicked.AddDynamic(this, &UM_ChatWidget::OnSendButtonClicked);
	}

	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UM_ChatWidget::OnInputTextCommitted);
	}
}

void UM_ChatWidget::NativeDestruct()
{
	if (UM_ChatSubSystem* ChatSub = GetChatSubsystem())
	{
		ChatSub->OnChatMessageReceived.RemoveDynamic(this, &UM_ChatWidget::HandleMessageReceived);
	}

	Super::NativeDestruct();
}

UM_ChatSubSystem* UM_ChatWidget::GetChatSubsystem() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UM_ChatSubSystem>();
	}
	return nullptr;
}

void UM_ChatWidget::HandleMessageReceived(const FString& Username, const FString& Message, const FString& Timestamp)
{
	if (!MessageScrollBox)
	{
		return;
	}

	UTextBlock* NewLine = NewObject<UTextBlock>(this);
	NewLine->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Username, *Message)));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UM_BackendSubsystem* BackendSub = GI->GetSubsystem<UM_BackendSubsystem>())
		{
			if (Username == BackendSub->LoggedInUsername)
			{
				NewLine->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
			}
		}
	}

	MessageScrollBox->AddChild(NewLine);
	MessageScrollBox->ScrollToEnd();
}

void UM_ChatWidget::OnSendButtonClicked()
{
	SendCurrentInput();
}

void UM_ChatWidget::OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendCurrentInput();
		UnfocusInput();
	}
}

void UM_ChatWidget::SendCurrentInput()
{

	if (!ChatInputBox)
	{
		UnfocusInput();
		return;
	}
	const FString MessageText = ChatInputBox->GetText().ToString();
	if (MessageText.TrimStartAndEnd().IsEmpty())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		UM_BackendSubsystem* BackendSub = GI->GetSubsystem<UM_BackendSubsystem>();
		UM_ChatSubSystem* ChatSub = GI->GetSubsystem<UM_ChatSubSystem>();

		if (BackendSub && ChatSub)
		{
			ChatSub->SendChatMessage(BackendSub->LoggedInUsername, MessageText);
		}
	}

	ChatInputBox->SetText(FText::GetEmpty());
	UnfocusInput();   

}

void UM_ChatWidget::FocusInput()
{
	if (!ChatInputBox) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ChatInputBox->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}

void UM_ChatWidget::UnfocusInput()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(false);
		
	
}