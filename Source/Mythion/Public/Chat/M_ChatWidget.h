// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "M_ChatWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UEditableTextBox;
class UButton;
class UM_ChatSubSystem;


UCLASS()
class MYTHION_API UM_ChatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Chat | Unfocus")
    void FocusInput();
	
	UFUNCTION(BlueprintCallable, Category = "Chat | Unfocus")
	void UnfocusInput();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* MessageScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInputBox;

	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;

	UFUNCTION()
	void HandleMessageReceived(const FString& Username, const FString& Message, const FString& Timestamp);

	UFUNCTION()
	void OnSendButtonClicked();

	UFUNCTION()
	void OnInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void SendCurrentInput();




private:
	UM_ChatSubSystem* GetChatSubsystem() const;
	
};
