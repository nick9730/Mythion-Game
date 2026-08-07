// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Attributes/M_AttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Characters/BaseCharacter.h"
#include "Characters/PlayerCharacter/M_PlayerState.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/WidgetComponent.h"
#include "DataAsset/CharacterClasses.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayEffectTypes.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "M_PlayerController.h"
#include "M_QuestComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Simple_Inventory/Data/M_Interactive_Item.h"
#include "Simple_Inventory/InventoryComponent.h"
#include "Weapons/WeaponBase.h"

APlayerCharacter::APlayerCharacter()
{

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    SpringArmComp->SetupAttachment(GetRootComponent());
    SpringArmComp->TargetArmLength = 300.f;
    SpringArmComp->bUsePawnControlRotation = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    Camera->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    QuestComponent = CreateDefaultSubobject<UM_QuestComponent>(TEXT("QuestComponent"));

    bReplicates = true;
    GetCharacterMovement()->SetIsReplicated(true);
    bUseControllerRotationYaw = true;

    GetCharacterMovement()->bOrientRotationToMovement = false;

    NameplateComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameplateComponent"));
    NameplateComponent->SetupAttachment(RootComponent);
    NameplateComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
    NameplateComponent->SetWidgetSpace(EWidgetSpace::Screen);
    NameplateComponent->SetDrawSize(FVector2D(200.f, 40.f));

    MiniMapCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MiniMapCaptureComponent"));
    MiniMapCaptureComponent->SetupAttachment(GetRootComponent());
    MiniMapCaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, 2000.f));
    MiniMapCaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
    MiniMapCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    MiniMapCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    MiniMapCaptureComponent->bCaptureEveryFrame = true;
}

void APlayerCharacter::OnRep_PlayerClassTag()
{
    ApplyCharacterClassData(CharacterClassData, PlayerClassTag);
}

void APlayerCharacter::Server_DestroyEquippedItems_Implementation()
{
    if (IsValid(EquippedWeapon))
        EquippedWeapon->Destroy();
    if (IsValid(EquippedArmor))
        EquippedArmor->Destroy();
}

// Weapon Equipped Warrior

void APlayerCharacter::UnequipWeapon(FItemData WeaponItem)
{

    UAbilitySystemComponent *ASC = GetAbilitySystemComponent();

    if (WeaponItem.ItemType == EItemType::Weapon)
    {
        if (IsValid(ASC) && ActiveWeaponEffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(ActiveWeaponEffectHandle);
            ActiveWeaponEffectHandle.Invalidate();
        }
        if (IsValid(EquippedWeapon))
        {
            EquippedWeapon->Destroy();
            EquippedWeapon = nullptr;
        }
    }
    else if (WeaponItem.ItemType == EItemType::Armor)
    {
        if (IsValid(ASC) && ActiveArmorEffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(ActiveArmorEffectHandle);
            ActiveArmorEffectHandle.Invalidate();
        }
        if (IsValid(EquippedArmor))
        {
            EquippedArmor->Destroy();
            EquippedArmor = nullptr;
        }
    }
}

void APlayerCharacter::OnWeaponEquipped(FItemData Item)
{
    UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return;
    if (Item.ItemType == EItemType::Weapon)
    {
        if (IsValid(Item.ItemDataAsset) && IsValid(Item.ItemDataAsset->UseEffect))
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Item.ItemDataAsset->UseEffect, 1, Context);
            ActiveWeaponEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }

        Server_SpawnAndEquipWeapon(Item);
    }
    else if ((Item.ItemType == EItemType::Armor))
    {
        if (IsValid(Item.ItemDataAsset) && IsValid(Item.ItemDataAsset->UseEffect))
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Item.ItemDataAsset->UseEffect, 1, Context);
            ActiveArmorEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
        }
        Server_SpawnAndEquipWeapon(Item);
    }
}

void APlayerCharacter::Server_SpawnAndEquipWeapon_Implementation(FItemData Item)
{

    if (!Item.ItemDataAsset)
        return;
    if (!Item.ItemDataAsset->WeaponClass)
        return;

    // Spawn νέο
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;

    if (Item.ItemType == EItemType::Weapon)
    {

        EquippedWeapon =
            GetWorld()->SpawnActor<AWeaponBase>(Item.ItemDataAsset->WeaponClass, GetActorTransform(), Params);

        if (!IsValid(EquippedWeapon))
            return;

        EquippedWeapon->WeaponDamage = Item.ItemDataAsset->Damage;

        if (!IsValid(EquippedWeapon))
            return;

        EquippedWeapon->WeaponDamage = Item.ItemDataAsset->Damage;

        Multicast_AttachWeapon(EquippedWeapon, Item);

        if (Item.ItemDataAsset->StaticMesh)
            EquippedWeapon->SetWeaponMesh(Item.ItemDataAsset->StaticMesh);
    }
    else if (Item.ItemType == EItemType::Armor)
    {

        EquippedArmor =
            GetWorld()->SpawnActor<AWeaponBase>(Item.ItemDataAsset->WeaponClass, GetActorTransform(), Params);

        if (!IsValid(EquippedArmor))
            return;

        if (Item.ItemDataAsset->StaticMesh)
            EquippedArmor->SetWeaponMesh(Item.ItemDataAsset->StaticMesh);

        Multicast_AttachWeapon(EquippedArmor, Item);
    }
}

void APlayerCharacter::Multicast_AttachWeapon_Implementation(AWeaponBase *Weapon, FItemData Item)
{

    if (Item.ItemType == EItemType::Weapon)
    {
        if (!IsValid(Weapon))
            return;
        FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
        Weapon->AttachToComponent(GetMesh(), Rules, Item.Socket);
    }
    else if (Item.ItemType == EItemType::Armor)
    {
        if (!IsValid(Weapon))
            return;
        FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
        Weapon->AttachToComponent(GetMesh(), Rules, Item.Socket);
    }
}

FVector2D APlayerCharacter::GetFullMapIconPosition(FVector2D ImageSize) const
{
    const float MinX = -20495.23f, MaxX = 15355.93f;
    const float MinY = -12130.20f, MaxY = 11334.46f;

    FVector Loc = GetActorLocation();

    float NormalizedX = (Loc.X - MinX) / (MaxX - MinX);
    float NormalizedY = (Loc.Y - MinY) / (MaxY - MinY);

    return FVector2D(NormalizedX * ImageSize.X, (1.f - NormalizedY) * ImageSize.Y);
}

// Replication and Initialization
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // if (HasAuthority()) {	if (WeaponData)	{SpawnWeapon();}}
    Tags.Add(FName("Player"));

    FString MapName = GetWorld()->GetMapName();

    if (MapName.Contains("Character_Selection"))
    {
        FRotator FixedRotation = FRotator(0.f, 0.f, 0.f);
        SetActorRotation(FixedRotation);
        bUseControllerRotationYaw = false;
    }

    AM_PlayerController *PC = Cast<AM_PlayerController>(GetController());

    if (IsLocallyControlled())
    {

        if (!MapName.Contains("Character_Selection") && !MapName.Contains("LoginLayer"))
        {
            if (IsValid(PC))
            {

                PC->ShowLoadingScreen();
            }
        }

        MiniMapRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, MiniMapResolution, MiniMapResolution,
                                                                            ETextureRenderTargetFormat::RTF_RGBA8);

        if (IsValid(MiniMapCaptureComponent) && IsValid(MiniMapRenderTarget))
        {
            MiniMapCaptureComponent->TextureTarget = MiniMapRenderTarget;
            MiniMapCaptureComponent->OrthoWidth = MiniMapOrthoWidth;

            if (IsValid(MiniMapBaseMaterial))
            {
                MiniMapDynamicMaterial = UMaterialInstanceDynamic::Create(MiniMapBaseMaterial, this);
                MiniMapDynamicMaterial->SetTextureParameterValue(FName("RenderTarget"), MiniMapRenderTarget);
            }

            OnMiniMapReady.Broadcast(MiniMapRenderTarget);
        }
    }

    if (NameplateWidgetClass && NameplateComponent)
    {
        NameplateComponent->SetWidgetClass(NameplateWidgetClass);
        NameplateComponent->InitWidget();
    }

    if (IsValid(InventoryComponent))
    {
        InventoryComponent->OnWeaponEquipped.RemoveAll(this);
        InventoryComponent->OnWeaponEquipped.AddDynamic(this, &APlayerCharacter::OnWeaponEquipped);
    }

    if (HasAuthority())
    {
        GrantCommonAbilities();
    }
    /*
    if (IsValid(InventoryComponent))
    {
        InventoryComponent->OnArmorEquipped.RemoveAll(this);
        InventoryComponent->OnArmorEquipped.AddDynamic(this, &APlayerCharacter::OnArmorEquipped);
    }
    */

    /*
        UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
        if (IsValid(ASC))
        {
            ASC->RegisterGameplayTagEvent(
                FGameplayTag::RequestGameplayTag("Status.Dead")
            ).AddUObject(this, &APlayerCharacter::OnDeathTagChanged);
        }
    */
}

void APlayerCharacter::GrantCommonAbilities()
{

    if (!IsValid(CommonAbilitiesData))
        return;

    AM_PlayerState *PlayState = Cast<AM_PlayerState>(GetPlayerState());
    UAbilitySystemComponent *ASC = PlayState ? PlayState->GetAbilitySystemComponent() : nullptr;

    if (!IsValid(ASC))
    {
        FTimerHandle RetryHandle;
        GetWorld()->GetTimerManager().SetTimer(RetryHandle, this, &APlayerCharacter::GrantCommonAbilities, 0.2f, false);
        return;
    }

    for (const FGrantCommonAbilitiesData &Entry : CommonAbilitiesData->CommonAbilities)
    {
        if (!IsValid(Entry.Ability))
            continue;

        ASC->GiveAbility(FGameplayAbilitySpec(Entry.Ability, 1, Entry.InputID));
    }
}

void APlayerCharacter::PossessedBy(AController *NewController)
{
    Super::PossessedBy(NewController);
    AM_PlayerState *PS = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PS))
        return;
    PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

    Multicast_ApplyCharacterClass(PlayerClassTag);
    Tags.Add(FName("Player"));
}

// State,Attribute,ASC

void APlayerCharacter::OnRep_PlayerState()
{
    AM_PlayerState *PS = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PS))
        return;

    PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

    PS->GetAbilitySystemComponent()->ForceReplication();

    PS->GetAttributeSet()->OnLevelUp.AddDynamic(this, &APlayerCharacter::LevelUp);

    AM_PlayerController *PC = Cast<AM_PlayerController>(GetController());
    if (IsValid(PC) && IsLocallyControlled())
    {
        PC->TryInitInventory();
        UM_BackendSubsystem *Backend = PC->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
    }

    if (IsValid(PC) && IsLocallyControlled())
    {
        if (IsValid(PC->QuantityWidgetClass) && !IsValid(PC->QuantityWidget))
        {
            PC->QuantityWidget = CreateWidget<UM_QuantityWidget>(PC, PC->QuantityWidgetClass);
            PC->QuantityWidget->AddToViewport(15);
            PC->QuantityWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (IsLocallyControlled())
    {
        FVector PreviewLocation = FVector(0.f, 0.f, -10000.f);
        PreviewActor = GetWorld()->SpawnActor<AM_PreviewActorInventory>(AM_PreviewActorInventory::StaticClass(),
                                                                        PreviewLocation, FRotator::ZeroRotator);

        if (IsValid(PreviewActor))
        {
            PreviewActor->InitPreview(GetMesh()->GetSkeletalMeshAsset(), PreviewAnimClass);
            OnPreviewReady.Broadcast(PreviewActor->GetRenderTarget());
        }

        AM_PlayerController *PController = Cast<AM_PlayerController>(GetController());
        if (IsValid(PController) && IsLocallyControlled())
        {
            UM_BackendSubsystem *Backend = PController->GetGameInstance()->GetSubsystem<UM_BackendSubsystem>();
            if (IsValid(Backend) && !Backend->AuthToken.IsEmpty())
            {
                PController->PlayerAuthToken = Backend->AuthToken;
                PController->LoadPlayerData();
            }
        }
    }

    /*
     *//**/
    FTimerHandle MeshTimer;
    GetWorld()->GetTimerManager().SetTimer(
        MeshTimer, [this]() { ApplyCharacterClassData(CharacterClassData, PlayerClassTag); }, 2.0f, false);

    OnASCInitialized.Broadcast(PS->GetAbilitySystemComponent(), Cast<UAttributeSet>(PS->GetAttributeSet()));
    if (IsValid(InventoryComponent))
    {
        InventoryComponent->OnWeaponEquipped.RemoveAll(this);
        InventoryComponent->OnWeaponEquipped.AddDynamic(this, &APlayerCharacter::OnWeaponEquipped);
    }
    /*
    if (IsLocallyControlled())
    {

        if (IsValid(PC))
        {

            PC->GetWorld()->GetTimerManager().SetTimer(
                AutoSaveTimer,
                PC,
                &AM_PlayerController::SaveInventory,
                60.f,
                true
            );
        }
    }
    */
}

UAbilitySystemComponent *APlayerCharacter::GetAbilitySystemComponent() const
{
    AM_PlayerState *PS = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PS))
        return nullptr;

    return PS->GetAbilitySystemComponent();
}

UAttributeSet *APlayerCharacter::GetAttributeSet() const
{
    AM_PlayerState *PS = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PS))
        return nullptr;
    return PS->GetAttributeSet();
}

// Replicate
void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APlayerCharacter, PlayerClassTag);
    DOREPLIFETIME(APlayerCharacter, DeathLocation);
}

void APlayerCharacter::Multicast_ApplyCharacterClass_Implementation(FGameplayTag ClassTag)
{
    ApplyCharacterClassData(CharacterClassData, ClassTag);
}

void APlayerCharacter::Server_ApplyCharacterClass_Implementation(FGameplayTag ClassTag)
{
    PlayerClassTag = ClassTag;
    ForceNetUpdate();
    Multicast_ApplyCharacterClass(ClassTag);
}

// SetupInitial Attributes and mesh
void APlayerCharacter::ApplyCharacterClassData(TSoftObjectPtr<UCharacterClasses> ClassData,
                                               const FGameplayTag TagMatches)
{
    USkeletalMeshComponent *MeshComp = GetMesh();
    if (!MeshComp)
        return;
    if (!TagMatches.IsValid())
        return;

    UCharacterClasses *LoadedClassData = ClassData.LoadSynchronous();
    if (!IsValid(LoadedClassData))
        return;

    for (const FCharacterClassData &Choice : LoadedClassData->CharacterClasses)
    {
        if (Choice.ClassNameTag.MatchesTagExact(TagMatches))
        {
            if (Choice.CharacterMesh)
                MeshComp->SetSkeletalMesh(Choice.CharacterMesh);

            MeshComp->SetAnimInstanceClass(Choice.AnimBlueprintClass);

            UAbilitySystemComponent *ASC = GetAbilitySystemComponent();
            if (IsValid(ASC))
            {
                FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
                if (IsValid(Choice.ManaRegenPerType))
                {
                    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Choice.ManaRegenPerType, 1, Context);
                    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                }
                if (IsValid(Choice.HealthRegenPerType))
                {
                    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Choice.HealthRegenPerType, 1, Context);
                    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                }
                if (IsValid(Choice.UpdateStatsPerType))
                {
                    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Choice.UpdateStatsPerType, 1, Context);
                    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                    LevelUpStats = Choice.UpdateStatsPerType;
                }
                if (IsValid(Choice.EnergyRegenPerType))
                {
                    FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Choice.EnergyRegenPerType, 1, Context);
                    ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                }
            }
            return;
        }
    }
}

void APlayerCharacter::HandleDeath()
{
    if (HasAuthority())
    {
        DeathLocation = GetActorLocation();
    }

    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    HandleRespawn();
}
void APlayerCharacter::HandleRespawn()
{
    AM_PlayerController *PC = Cast<AM_PlayerController>(GetController());
    if (IsValid(PC))
    {
        PC->Client_ShowRespawnWidget();
    }
}
void APlayerCharacter::OnDeathTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
    if (NewCount > 0)
    {
        // Handle death logic here

        HandleDeath();
    }
    else
    {
        // Handle respawn logic here
        UE_LOG(LogTemp, Warning, TEXT("Player has respawned."));
    }
}

void APlayerCharacter::SetupAttributes()
{

    if (!HasAuthority())
        return;

    AM_PlayerState *PlayState = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PlayState))
        return;
    AM_PlayerState *PS = Cast<AM_PlayerState>(GetPlayerState());
    UAbilitySystemComponent *ASC = PlayState->GetAbilitySystemComponent();
    if (!IsValid(PS))
        return;

    for (const TSubclassOf<UGameplayAbility> &Ability : CommonAbilities)
    {

        if (IsValid(Ability))
        {
            ASC->GiveAbility(FGameplayAbilitySpec(Ability, 1));
        }
    }

    for (const FCharacterClassData &ClassData : CharacterClassData->CharacterClasses)
    {
        if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag))
            continue;
        FString MapName = GetWorld()->GetMapName();

        if (!MapName.Contains("MythionMap") && !MapName.Contains("Untitledeeee"))
            return;

        for (const FGrantAbilitiesDataByLevel &AbilityEntry : ClassData.AbilitiesGrantedByLevel)
        {
            float CurrentLevel = GetAbilitySystemComponent()->GetNumericAttribute(UM_AttributeSet::GetLevelAttribute());
            if (AbilityEntry.RequiredLevel <= FMath::FloorToInt(CurrentLevel))
            {
                GetAbilitySystemComponent()->GiveAbility(
                    FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1, AbilityEntry.InputID));
                // OnAbilityGranted.Broadcast(AbilityEntry);
                Client_OnAbilityGranted(AbilityEntry);
            }
        }
    }

    /*
        if (InitializeAttributesEffect)
        {
            InitializeAttributes(InitializeAttributesEffect);
        }
    */

    ASC->GetGameplayAttributeValueChangeDelegate(UM_AttributeSet::GetLevelAttribute())
        .AddUObject(this, &APlayerCharacter::OnLevelAttributeChanged);

    OnASCInitialized.Broadcast(PS->GetAbilitySystemComponent(), Cast<UAttributeSet>(PS->GetAttributeSet()));

    bASCInitialized = true;
}
void APlayerCharacter::OnLevelAttributeChanged(const FOnAttributeChangeData &Data)
{

    LevelUp(Data.NewValue);
}
void APlayerCharacter::OnRep_PlayerName()
{
}

// Levels and Abilities
void APlayerCharacter::LevelUp(float NewLevel)
{

    if (IsValid(CharacterClassData))
    {
        for (const FCharacterClassData &ClassData : CharacterClassData->CharacterClasses)
        {
            if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag))
                continue;

            for (const FGrantAbilitiesDataByLevel &AbilityEntry : ClassData.AbilitiesGrantedByLevel)
            {
                if (AbilityEntry.RequiredLevel == FMath::FloorToInt(NewLevel))
                {

                    GetAbilitySystemComponent()->GiveAbility(
                        FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1, AbilityEntry.InputID));

                    if (!AbilityEntry.bIsPassive)
                    {
                        Client_OnAbilityGranted(AbilityEntry);
                    }
                    OnAbilityInitialized.Broadcast(AbilityEntry);
                    OnLevelChanged.Broadcast(NewLevel);
                }
            }
        }
    }
}

void APlayerCharacter::GrantAbilities()
{
    if (!IsValid(CharacterClassData))
        return;
    AM_PlayerState *PlayState = Cast<AM_PlayerState>(GetPlayerState());
    if (!IsValid(PlayState))
        return;
    UAbilitySystemComponent *ASC = PlayState->GetAbilitySystemComponent();
    if (!IsValid(ASC))
        return; // koita edw an exeis thema
    AM_PlayerController *PC = Cast<AM_PlayerController>(GetController());
    if (!IsValid(PC))
        return;
    FString MapName = GetWorld()->GetMapName();

    if (!MapName.Contains("MythionMap") && !MapName.Contains("Untitledeeee"))
        return;
    for (const FCharacterClassData &ClassData : CharacterClassData->CharacterClasses)
    {
        if (!ClassData.ClassNameTag.MatchesTagExact(PlayerClassTag))
            continue;

        for (const FGrantAbilitiesDataByLevel &AbilityEntry : ClassData.AbilitiesGrantedByLevel)
        {
            if (IsValid(AbilityEntry.AbilityToGrant) &&
                AbilityEntry.RequiredLevel <= FMath::FloorToInt(GetAbilitySystemComponent()->GetNumericAttribute(
                                                  UM_AttributeSet::GetLevelAttribute())))
            {
                ASC->GiveAbility(FGameplayAbilitySpec(AbilityEntry.AbilityToGrant, 1, AbilityEntry.InputID));
            }
        }
    }
}

void APlayerCharacter::Client_OnAbilityGranted_Implementation(FGrantAbilitiesDataByLevel AbilityEntry)
{
    OnAbilityGranted.Broadcast(AbilityEntry);
}
