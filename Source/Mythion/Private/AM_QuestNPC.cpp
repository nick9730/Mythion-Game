// Fill out your copyright notice in the Description page of Project Settings.


#include "AM_QuestNPC.h"
#include "Components/StaticMeshComponent.h" 
#include "DataAsset/M_QuestDataAsset.h"
#include "Characters/PlayerCharacter.h"
#include "M_QuestComponent.h"
#include "M_PlayerController.h"
#include "M_QuestStatus.h"
#include "M_QuestController.h"
#include "M_QuestDialolgWidget.h"
#include "GameFramework/Character.h"





AAM_QuestNPC::AAM_QuestNPC()
{
    PrimaryActorTick.bCanEverTick = true;
    SetReplicateMovement(true);
	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(RootComponent);
	QuestStatusIcon = CreateDefaultSubobject<UWidgetComponent>(TEXT("QuestStatusIcon"));
    QuestStatusIcon->SetupAttachment(RootComponent);
    QuestStatusIcon->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    QuestStatusIcon->SetWidgetSpace(EWidgetSpace::Screen);
    QuestStatusIcon->SetDrawSize(FVector2D(100.f, 100.f));



}
void AAM_QuestNPC::BeginPlay()
{

    Super::BeginPlay();

    if (!IsValid(QuestDataAsset)) return;
    for (const FQuestData& Quest : QuestDataAsset->Quests)
    {
        if (Quest.QuestID == QuestID)
        {
            CurrentQuestData = Quest;
            break;
        }
    }
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!IsValid(PC)) return;


    if (IsValid(QuestStatusIcon) && IsValid(QuestStatusIconClass))
    {
        QuestStatusIcon->SetWidgetClass(QuestStatusIconClass);
        QuestStatusIcon->InitWidget();
        QuestStatusStatusIcon = Cast<UM_QuestStatus>(QuestStatusIcon->GetUserWidgetObject());
    }
    QuestStatusStatusIcon = Cast<UM_QuestStatus>(QuestStatusIcon->GetUserWidgetObject());
    
}
void AAM_QuestNPC::Interact(AActor* Caller)
{

    APlayerCharacter* Player = Cast<APlayerCharacter>(Caller);
    if (!IsValid(Player) || !IsValid(QuestDataAsset)) return;



    UM_QuestComponent* QuestComp = Player->FindComponentByClass<UM_QuestComponent>();



    if (IsValid(QuestComp))
    {
        if (QuestComp->CompletedQuestIDs.Contains(NPCQuestID))
        {
            AM_PlayerController* PC = Cast<AM_PlayerController>(Player->GetController());
            if (IsValid(PC))
                PC->Client_ShowNotification(FText::FromString(TEXT("Quest already completed!")));
            return;
        }

        if (QuestComp->ActiveQuestAssets.Num() > 0)
        {
            AM_PlayerController* PC = Cast<AM_PlayerController>(Player->GetController());
            if (IsValid(PC))
                PC->Client_ShowNotification(FText::FromString(TEXT("You already have an active quest!")));
            return;
        }
    }
    InteractingPlayer = Player;



    AM_PlayerController* PC = Cast<AM_PlayerController>(Player->GetController());
    if (!IsValid(PC)) return;

    PC->Client_ShowQuestDialog(this, CurrentQuestData);
    
}
void AAM_QuestNPC::UpdateQuestStatusIcon(EQuestStatus Status)
{
   

   QuestStatusStatusIcon = Cast<UM_QuestStatus>(QuestStatusIcon->GetUserWidgetObject());

    if (!IsValid(QuestStatusStatusIcon)) return;

     QuestStatusStatusIcon->UpdateQuestStatusImage(Status);
}
void AAM_QuestNPC::OnFocused()
{
    if (IsValid(CharacterMesh) && IsValid(HighlightMaterial))
        CharacterMesh->SetMaterial(0, HighlightMaterial);
}
void AAM_QuestNPC::OnUnfocused()
{
    if (IsValid(CharacterMesh) && IsValid(OriginalMaterial))
        CharacterMesh->SetMaterial(0, OriginalMaterial);
}






















 //   CreateQuestDialogWidget(PC);



    /*
	UM_QuestComponent* QuestComp = Player->FindComponentByClass<UM_QuestComponent>();
	if (IsValid(QuestComp))
		QuestComp->QuestNPCs.Add(CurrentQuestData.QuestID, this);

	PC->ShowQuestDialog(this,CurrentQuestData);
    */




/*
void AAM_QuestNPC::BeginPlay()
{
	if (!IsValid(QuestDataAsset)) return;

    Super::BeginPlay();

 for (const FQuestData& Quest : QuestDataAsset->Quests)
    {
        if (Quest.QuestID == QuestID)
        {
            CurrentQuestData = Quest;
            break;
        }
    }
 APlayerController* PC = GetWorld()->GetFirstPlayerController();
 if (!IsValid(PC)) return;


}
*/
 /*

 if (QuestStatusIconClass && QuestStatusStatusIcon)
 {
     QuestStatusStatusIcon->SetWidgetClass(QuestStatusIconClass);
     QuestStatusStatusIcon->InitWidget();

     QuestStatusStatusIcon = Cast<UM_QuestStatus>(QuestStatusStatusIcon->GetUserWidgetObject());
     if (IsValid(QuestStatusStatusIcon))
         QuestStatusStatusIcon->UpdateQuestStatusImage(CurrentQuestData);
 }
 */

 /*
 QuestStatusUserWidget = Cast<UM_QuestStatus>(QuestStatusWidget->GetUserWidgetObject());
 if (!IsValid(QuestStatusUserWidget)) return;

 QuestStatusUserWidget->UpdateQuestStatusImage(CurrentQuestData);
 QuestStatusUserWidget = CreateWidget<UM_QuestStatus>(PC, QuestStatusWidgetClass);
 if (!IsValid(QuestStatusUserWidget)) return;
 QuestStatusUserWidget->AddToViewport();
 QuestStatusUserWidget->UpdateQuestStatusImage(CurrentQuestData);


 /*
 QuestStatusWidget->SetWidgetClass(QuestStatusWidgetClass);
 QuestStatusWidget->InitWidget();

 QuestStatusUserWidget = Cast<UM_QuestStatus>(QuestStatusWidget->GetUserWidgetObject());
 if (!IsValid(QuestStatusUserWidget)) return;


 QuestStatusUserWidget->UpdateQuestStatusImage(CurrentQuestData);
 */


void AAM_QuestNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);



    if (!IsValid(InteractingPlayer))	 return;
    float Distance = FVector::Dist(GetActorLocation(), InteractingPlayer->GetActorLocation());
    if (Distance > 500.f)
    {
        AM_PlayerController* PC = Cast<AM_PlayerController>(InteractingPlayer->GetController());
        if (!IsValid(PC)) return;
        if (IsValid(PC->QuestWidgetDialog) && PC->QuestWidgetDialog->GetIsDialogOpen())
        {
            PC->QuestWidgetDialog->CloseDialog();
        }

        InteractingPlayer = nullptr;
    }
}
    /*
    if (!IsValid(QuestStatusUserWidget)) return;
   
    APlayerController* LocalPC = nullptr;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (IsValid(PC) && PC->IsLocalController())
        {
            LocalPC = PC;
            break;
        }
    }

    if (!IsValid(LocalPC)) return;

    APawn* PlayerPawn = LocalPC->GetPawn();
    if (!IsValid(PlayerPawn)) return;

    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Distance > InteractionDistance)
    {
        QuestStatusUserWidget->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    QuestStatusUserWidget->SetVisibility(ESlateVisibility::Visible);

    FVector HeadLocation = GetActorLocation() + FVector(0.f, 0.f, 60.f);
    FVector2D ScreenPos;

    

    if (LocalPC->ProjectWorldLocationToScreen(HeadLocation, ScreenPos))
    {
        QuestStatusUserWidget->SetPositionInViewport(ScreenPos - FVector2D(25.f, 25.f));
    }
    */

/*

  if (QuestStatusIconClass && QuestStatusStatusIcon)
  {
      QuestStatusStatusIcon->SetWidgetClass(QuestStatusIconClass);
      QuestStatusStatusIcon->InitWidget();

      QuestStatusStatusIcon = Cast<UM_QuestStatus>(QuestStatusStatusIcon->GetUserWidgetObject());
      if (IsValid(QuestStatusStatusIcon))
          QuestStatusStatusIcon->UpdateQuestStatusImage(CurrentQuestData);
  }
  */