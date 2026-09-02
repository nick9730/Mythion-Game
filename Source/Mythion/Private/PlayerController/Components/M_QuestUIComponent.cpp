// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerController/Components/M_QuestUIComponent.h"
#include "AM_QuestNPC.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "M_CompletedQuestWidget.h"
#include "M_PlayerController.h"
#include "M_QuestComponent.h"
#include "M_QuestDialolgWidget.h"
#include "M_QuestsInfos.h"
#include "PlayerController/Components/M_BackendComponent.h"
#include "Widgets/PlayerWidget/StatsWidget.h"

UM_QuestUIComponent::UM_QuestUIComponent()
{

    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}

void UM_QuestUIComponent::Client_ShowQuestDialog_Implementation(AAM_QuestNPC *NPC, FQuestData QuestData)
{
    if (!IsValid(QuestWidgetDialogClass))
        return;

    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (!IsValid(QuestWidgetDialog))
    {
        QuestWidgetDialog = CreateWidget<UM_QuestDialolgWidget>(OwnerPC, QuestWidgetDialogClass);
        if (!IsValid(QuestWidgetDialog))
            return;
        QuestWidgetDialog->AddToViewport();
        QuestWidgetDialog->SetVisibility(ESlateVisibility::Hidden);
    }

    QuestWidgetDialog->InitDialog(NPC, QuestData);

    if (QuestWidgetDialog->GetIsDialogOpen())
        QuestWidgetDialog->CloseDialog();
    else
        QuestWidgetDialog->OpenDialog();
}

void UM_QuestUIComponent::Client_PendingQuestLoaded_Implementation(FQuestData QuestAsset)
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC) || !OwnerPC->IsLocalController())
        return;

    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerPC);
    if (!IsValid(PCController) || !IsValid(PCController->StatsWidget) ||
        !IsValid(PCController->StatsWidget->QuestInfoWidget))
        return;

    UM_QuestsInfos *QuestInfoWidgeOpen = PCController->StatsWidget->QuestInfoWidget;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerPC->GetPawn());
    if (IsValid(PC))
    {
        UM_QuestComponent *QuestComp = PC->FindComponentByClass<UM_QuestComponent>();
        if (IsValid(QuestComp))
        {
            QuestComp->OnQuestUpdated.RemoveDynamic(QuestInfoWidgeOpen, &UM_QuestsInfos::UpdateQuest);
            QuestComp->OnQuestUpdated.AddDynamic(QuestInfoWidgeOpen, &UM_QuestsInfos::UpdateQuest);
        }
    }

    QuestInfoWidgeOpen->UpdateQuest(QuestAsset, 0);
    QuestInfoWidgeOpen->OpenQuestInfo();
    UpdateNPCQuestStatus(QuestAsset, EQuestStatus::Pending);

    if (IsValid(PCController) && IsValid(PCController->BackendComponent))
    {
        PCController->BackendComponent->SavePlayerQuestsForBackEnd(QuestAsset);
    }
}

void UM_QuestUIComponent::UpdateNPCQuestStatus(FQuestData Quest, EQuestStatus Status)
{
    TArray<AActor *> FoundNPCs;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAM_QuestNPC::StaticClass(), FoundNPCs);

    if (FoundNPCs.Num() == 0)
    {
        FQuestData QuestCopy = Quest;
        TWeakObjectPtr<UM_QuestUIComponent> WeakThis(this);
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(
            RetryHandle,
            [WeakThis, QuestCopy, Status]() {
                if (WeakThis.IsValid())
                    WeakThis->UpdateNPCQuestStatus(QuestCopy, Status);
            },
            0.5f, false);
        return;
    }

    for (AActor *Actor : FoundNPCs)
    {
        AAM_QuestNPC *NPC = Cast<AAM_QuestNPC>(Actor);
        if (IsValid(NPC) && NPC->NPCQuestID.Equals(Quest.QuestID))
        {
            NPC->UpdateQuestStatusIcon(Status);
            break;
        }
    }
}

void UM_QuestUIComponent::ShowCompletedQuest(FQuestData Quest)
{
    APlayerController *OwnerPC = Cast<APlayerController>(GetOwner());
    if (!IsValid(OwnerPC))
        return;

    if (!IsValid(QuestCompletedWidget))
    {
        QuestCompletedWidget = CreateWidget<UM_CompletedQuestWidget>(OwnerPC, QuestCompletedClass);
        if (!IsValid(QuestCompletedWidget))
            return;
        QuestCompletedWidget->AddToViewport(10);
    }
    QuestCompletedWidget->SetText(Quest);
    QuestCompletedWidget->SetVisibility(ESlateVisibility::Visible);

    TWeakObjectPtr<UM_QuestUIComponent> WeakThis(this);
    FTimerHandle HideTimer;
    GetWorld()->GetTimerManager().SetTimer(
        HideTimer,
        [WeakThis]() {
            if (WeakThis.IsValid() && IsValid(WeakThis->QuestCompletedWidget))
                WeakThis->QuestCompletedWidget->SetVisibility(ESlateVisibility::Hidden);
        },
        4.f, false);
}

void UM_QuestUIComponent::Server_CompleteQuest_Implementation(FQuestData QuestAsset)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    if (!IsValid(OwnerController))
        return;

    APlayerCharacter *PC = Cast<APlayerCharacter>(OwnerController->GetPawn());
    if (!IsValid(PC))
        return;

    UM_QuestComponent *QuestComp = PC->FindComponentByClass<UM_QuestComponent>();
    if (!IsValid(QuestComp))
        return;

    AM_PlayerState *PS = Cast<AM_PlayerState>(PC->GetPlayerState());
    if (!IsValid(PS))
        return;

    UAbilitySystemComponent *ASC = PS->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;

    FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CoinEffect, 1, Context);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
        Spec, FGameplayTag::RequestGameplayTag("Overlap.Coins"), QuestAsset.CoinReward);
    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UM_QuestUIComponent::Client_CompleteQuest_Implementation(FQuestData QuestAsset)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

    if (IsValid(PCController) && IsValid(PCController->StatsWidget) &&
        IsValid(PCController->StatsWidget->QuestInfoWidget))
    {
        PCController->StatsWidget->QuestInfoWidget->CloseQuestInfo();
    }

    UpdateNPCQuestStatus(QuestAsset, EQuestStatus::Completed);

    if (IsValid(PCController) && IsValid(PCController->BackendComponent))
    {
        PCController->BackendComponent->SavePlayerQuestsForBackEnd(QuestAsset);
    }

    ShowCompletedQuest(QuestAsset);
}

void UM_QuestUIComponent::Client_UpdateKills_Implementation(FQuestData Quest, int32 CurrentKills)
{
    AController *OwnerController = Cast<AController>(GetOwner());
    AM_PlayerController *PCController = Cast<AM_PlayerController>(OwnerController);

    if (!IsValid(PCController) || !IsValid(PCController->StatsWidget) ||
        !IsValid(PCController->StatsWidget->QuestInfoWidget))
        return;

    PCController->StatsWidget->QuestInfoWidget->UpdateQuest(Quest, CurrentKills);
}