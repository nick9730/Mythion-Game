

#include "GAS/M_GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/M_Enemy_Controller.h"
#include "Characters/Enemy.h"
#include "Characters/PlayerCharacter.h"
#include "GAS/M_GameplayAbility.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Projectiles/M_BaseProjectile.h"
#include "Weapons/WeaponBase.h"

void UM_GameplayAbility::PlayMontage()
{
    if (!PlayableAnimMonage)
        return;

    UAbilityTask_PlayMontageAndWait *MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, PlayableAnimMonage, 1.0f, NAME_None, true, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UM_GameplayAbility::OnMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UM_GameplayAbility::OnMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UM_GameplayAbility::OnMontageCancelled);

    MontageTask->ReadyForActivation();
}

void UM_GameplayAbility::OnMontageCompleted()
{
    if (ExpectedInterruptCount > 0)
    {
        ExpectedInterruptCount--;
        return;
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UM_GameplayAbility::OnMontageInterrupted()
{

    if (ExpectedInterruptCount > 0)
    {
        ExpectedInterruptCount--;
        return;
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UM_GameplayAbility::OnMontageCancelled()
{
    if (ExpectedInterruptCount > 0)
    {
        ExpectedInterruptCount--;
        return;
    }
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UM_GameplayAbility::WaitGameplayEvent(FGameplayTag EventTag)
{

    UAbilityTask_WaitGameplayEvent *WaitEventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
    WaitEventTask->EventReceived.AddDynamic(this, &UM_GameplayAbility::OnGameplayEventReceived);
    WaitEventTask->ReadyForActivation();
}

void UM_GameplayAbility::OnGameplayEventReceived(FGameplayEventData Payload)
{
}

void UM_GameplayAbility::ApplyEffectWithMagnitude(AActor *TargetActor, FGameplayTag SetByCallerTag, float Magnitude)
{

    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (!IsValid(TargetActor) || !AssignedEffectClass)
        return;

    UAbilitySystemComponent *SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
        return;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return;

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle =
        SourceASC->MakeOutgoingSpec(AssignedEffectClass, GetAbilityLevel(), ContextHandle);

    if (SpecHandle.IsValid())
    {
        SpecHandle.Data->SetSetByCallerMagnitude(SetByCallerTag, Magnitude);
        TargetASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}

void UM_GameplayAbility::ApplyOutgoingEffect(AActor *TargetActor)
{
    if (!HasAuthority(&CurrentActivationInfo))
        return;

    if (!AssignedEffectClass)
        return;

    UAbilitySystemComponent *SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(SourceASC))
        return;

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle =
        SourceASC->MakeOutgoingSpec(AssignedEffectClass, GetAbilityLevel(), ContextHandle);

    if (SpecHandle.IsValid())
    {
        SourceASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UM_GameplayAbility::ExecuteCueOnActor(AActor *TargetActor, FGameplayTag CueTag, float RawMagnitude,
                                           FVector Location)
{
    if (!IsValid(TargetActor))
        return;

    UAbilitySystemComponent *TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (!IsValid(TargetASC))
        return;

    FGameplayCueParameters CueParams;
    CueParams.Location = Location;
    CueParams.RawMagnitude = RawMagnitude;
    CueParams.SourceObject = this;
    CueParams.Instigator = GetAvatarActorFromActorInfo();

    TargetASC->ExecuteGameplayCue(CueTag, CueParams);
}

void UM_GameplayAbility::ExecuteCueAtLocation(FGameplayTag CueTag, float RawMagnitude, FVector Location)
{
    UAbilitySystemComponent *ASC = GetAbilitySystemComponentFromActorInfo();
    if (!IsValid(ASC))
        return;

    FGameplayCueParameters CueParams;
    CueParams.Location = Location;
    CueParams.RawMagnitude = RawMagnitude;
    CueParams.SourceObject = this;
    CueParams.Instigator = GetAvatarActorFromActorInfo();

    ASC->ExecuteGameplayCue(CueTag, CueParams);
}

void UM_GameplayAbility::StartEventTimeoutSafety()
{
    UAbilityTask_WaitDelay *SafetyTimeout = UAbilityTask_WaitDelay::WaitDelay(this, EventTimeoutDuration);
    SafetyTimeout->OnFinish.AddDynamic(this, &UM_GameplayAbility::OnEventTimeout);
    SafetyTimeout->ReadyForActivation();
}

void UM_GameplayAbility::OnEventTimeout()
{

    if (!bHasEnded)
    {
        UE_LOG(LogTemp, Warning, TEXT("Ability %s timed out waiting for gameplay event - possible network issue"),
               *GetName());
        bHasEnded = true;
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}

void UM_GameplayAbility::ApplyAOEDamageAtLocation(FVector StartLocation, FVector EndLocation, float Radius,
                                                  float Damage, AActor *Instigator, int32 CurrentLevel,
                                                  FGameplayTag CueLocation)
{

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor *> IgnoreActors;
    IgnoreActors.Add(Instigator);

    TArray<FHitResult> OutHits;
    bool bHit =
        UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartLocation, EndLocation, Radius, ObjectTypes, false,
                                                         IgnoreActors, EDrawDebugTrace::None, OutHits, true);

    float Magnitude = Damage * CurrentLevel;
    FGameplayTag SetByCallerTag = FGameplayTag::RequestGameplayTag(FName("Data.Damage.Magical"));

    if (bHit)
    {
        for (const FHitResult &Hit : OutHits)
        {
            AActor *Actor = Hit.GetActor();
            AEnemy *Enemy = Cast<AEnemy>(Actor);
            APlayerCharacter *Character = Cast<APlayerCharacter>(Actor);

            if (IsValid(Enemy))
            {
                ApplyEffectWithMagnitude(Enemy, SetByCallerTag, Magnitude);
            }
            else if (IsValid(Character))
            {
                ApplyEffectWithMagnitude(Character, SetByCallerTag, Magnitude);
            }
        }
    }

    ExecuteCueOnActor(Instigator, CueLocation, Radius, StartLocation);
}

float UM_GameplayAbility::GetCharacterLevel(APlayerCharacter *Character)
{
    if (!IsValid(Character))
        return 1.0f;

    UAbilitySystemComponent *ASC = Character->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return 1.0f;
    const UM_AttributeSet *AttributeSet = ASC->GetSet<UM_AttributeSet>();
    float PlayerLevel = 1.0f;
    if (IsValid(AttributeSet))
    {
        PlayerLevel = AttributeSet->GetLevel();
    }

    return PlayerLevel;
}

FVector UM_GameplayAbility::PositionOfPlayerCharacter(AEnemy *EnemyCharacter)
{

    if (IsValid(EnemyCharacter))
    {
        if (!IsValid(EnemyCharacter))
            return FVector::ZeroVector;

        AM_Enemy_Controller *EnemyController = Cast<AM_Enemy_Controller>(EnemyCharacter->GetController());
        if (!IsValid(EnemyController))
            return FVector::ZeroVector;

        UBlackboardComponent *BlackboardComp = EnemyController->GetBlackboardComponent();
        if (!IsValid(BlackboardComp))
            return FVector::ZeroVector;

        AActor *TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKeyName));
        if (!IsValid(TargetActor))
            return FVector::ZeroVector;

        return TargetActor->GetActorLocation();
    }
    return FVector::ZeroVector;
}