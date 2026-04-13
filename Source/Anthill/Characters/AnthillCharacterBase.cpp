// Fill out your copyright notice in the Description page of Project Settings.


#include "AnthillCharacterBase.h"

#include "Anthill/AnthillGameInstance.h"
#include "Anthill/GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "Anthill/Pickups/AnthillPickupBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Anthill/UI/AnthillMenuLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace
{
static constexpr float InventoryValueTolerance = 0.001f;

static bool InventorySlotsStackTogether(const FInventorySlot& A, const FInventorySlot& B)
{
	return A.Type == B.Type
		&& FMath::IsNearlyEqual(A.Value1, B.Value1, InventoryValueTolerance)
		&& FMath::IsNearlyEqual(A.Value2, B.Value2, InventoryValueTolerance);
}
}

// Sets default values
AAnthillCharacterBase::AAnthillCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Add the ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);
	
	//set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.0f);
	
	//configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.0f);
	
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
	
	// Zestaw atrybutów (Health, Stamina) – subobject tej samej postaci co ASC
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));

	InventorySlots.SetNum(InventorySlotCount);
}

void AAnthillCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnthillCharacterBase, InventorySlots);
	DOREPLIFETIME(AAnthillCharacterBase, SelectedInventorySlotIndex);
}

// Called when the game starts or when spawned
void AAnthillCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && BasicAttributeSet)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BasicAttributeSet->GetHealthAttribute())
			.AddUObject(this, &AAnthillCharacterBase::HandleHealthChanged);
	}
}

void AAnthillCharacterBase::HandleHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	if (ChangeData.NewValue <= 0.f)
	{
		HandleDeathInternal();
	}
	else
	{
		bDeathHandled = false;
	}
}

void AAnthillCharacterBase::HandleDeathInternal()
{
	if (bDeathHandled)
	{
		return;
	}
	bDeathHandled = true;

	const bool bIsPlayerPawn = Cast<APlayerController>(GetController()) != nullptr;
	if (bOpenLoadMenuOnDeath && bIsPlayerPawn)
	{
		// Pozwalamy BP odpalić animację śmierci, a dopiero potem przechodzimy do Load Menu.
		OnDeath();

		if (APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0))
		{
			if (LocalPC->GetPawn() == this)
			{
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(LoadMenuAfterDeathTimerHandle);
					World->GetTimerManager().SetTimer(
						LoadMenuAfterDeathTimerHandle,
						this,
						&AAnthillCharacterBase::OpenLoadMenuAfterDeathDelay,
						FMath::Max(0.f, LoadMenuDelayOnDeathSeconds),
						false
					);
				}
				return;
			}
		}
	}

	OnDeath();
}

void AAnthillCharacterBase::OpenLoadMenuAfterDeathDelay()
{
	UAnthillMenuLibrary::OpenLoadMenuLevel(this);
}

void AAnthillCharacterBase::ApplyDefaultSpawnStatsOnly()
{
	if (bAnthillInitialStatsApplied || !BasicAttributeSet)
	{
		return;
	}

	const float MaxH = FMath::Max(0.f, DefaultMaxHealth);
	const float StartH = FMath::Clamp(MaxH * StartingHealthPercent, 0.f, MaxH);
	BasicAttributeSet->SetMaxHealth(MaxH);
	BasicAttributeSet->SetHealth(StartH);
	bAnthillInitialStatsApplied = true;
}

void AAnthillCharacterBase::ApplyInitialStatsAndLoadRestore()
{
	if (bAnthillInitialStatsApplied || !BasicAttributeSet || !GetWorld())
	{
		return;
	}

	UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(GetWorld()->GetGameInstance());
	FAnthillPendingLoadData Pending{};
	const bool bConsumed = GI && GI->ConsumePendingLoad(Pending);

	if (bConsumed && Pending.bHasCharacterState)
	{
		const float MaxH = FMath::Max(1.f, Pending.MaxHealth);
		const float MaxS = FMath::Max(1.f, Pending.MaxStamina);
		const float H = FMath::Clamp(Pending.Health, 0.f, MaxH);
		const float S = FMath::Clamp(Pending.Stamina, 0.f, MaxS);
		BasicAttributeSet->SetMaxHealth(MaxH);
		BasicAttributeSet->SetHealth(H);
		BasicAttributeSet->SetMaxStamina(MaxS);
		BasicAttributeSet->SetStamina(S);

		InventorySlots.SetNum(InventorySlotCount);
		for (int32 i = 0; i < InventorySlotCount; ++i)
		{
			InventorySlots[i] = Pending.InventorySlots.IsValidIndex(i) ? Pending.InventorySlots[i] : FInventorySlot();
		}
		SelectedInventorySlotIndex = FMath::Clamp(Pending.SelectedInventorySlotIndex, 0, InventorySlotCount - 1);
	}
	else
	{
		const float MaxH = FMath::Max(0.f, DefaultMaxHealth);
		const float StartH = FMath::Clamp(MaxH * StartingHealthPercent, 0.f, MaxH);
		BasicAttributeSet->SetMaxHealth(MaxH);
		BasicAttributeSet->SetHealth(StartH);
	}

	if (bConsumed && Pending.bHasTransform)
	{
		SetActorTransform(Pending.Transform, false, nullptr, ETeleportType::TeleportPhysics);
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->SetControlRotation(Pending.Transform.GetRotation().Rotator());
		}
	}

	bAnthillInitialStatsApplied = true;
}

// Called every frame
void AAnthillCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Fallback dla ścieżek obrażeń omijających delegate ASC (np. bezpośredni SetHealth w BP).
	if (BasicAttributeSet)
	{
		const float CurrentHealth = BasicAttributeSet->GetHealth();
		if (CurrentHealth <= 0.f)
		{
			HandleDeathInternal();
		}
		else
		{
			bDeathHandled = false;
		}
	}

	// Czas od ostatniego ataku (do opóźnienia regeneracji staminy i cooldownu ataku)
	TimeSinceLastAttack += DeltaTime;
	TimeSinceLastAttackTrigger += DeltaTime;

	// Serwerowo aktualizujemy atrybuty staminy.
	if (HasAuthority() && AbilitySystemComponent && BasicAttributeSet)
	{
		if (bIsSprinting)
		{
			TimeSinceSprintStopped = 0.f;

			const float CurrentStamina = BasicAttributeSet->GetStamina();
			if (CurrentStamina <= 0.f)
			{
				StopSprint();
				return;
			}

			// Stamina spada tylko gdy postać faktycznie się rusza (nie gdy stoi z wciśniętym Shift).
			const float SpeedSq = GetVelocity().SizeSquared();
			const float MinSpeedSq = 100.f * 100.f; // ~100 cm/s prędkości uznajemy za "stanie"
			if (SpeedSq >= MinSpeedSq)
			{
				const float Drain = StaminaDrainPerSecond * DeltaTime;
				AbilitySystemComponent->ApplyModToAttributeUnsafe(
					BasicAttributeSet->GetStaminaAttribute(),
					EGameplayModOp::Additive,
					-Drain
				);
			}
			else
			{
				// Stoi z wciśniętym Shift – odpoczynek; regen tylko gdy minęło opóźnienie po ataku
				if (TimeSinceLastAttack >= StaminaRegenDelayAfterAttackSeconds)
				{
					const float Regen = StaminaRegenPerSecond * DeltaTime;
					AbilitySystemComponent->ApplyModToAttributeUnsafe(
						BasicAttributeSet->GetStaminaAttribute(),
						EGameplayModOp::Additive,
						Regen
					);
				}
			}

			if (BasicAttributeSet->GetStamina() <= 0.f)
			{
				StopSprint();
			}
		}
		else
		{
			TimeSinceSprintStopped += DeltaTime;
			// Regen tylko gdy minęło opóźnienie po sprintcie i po ataku
			const bool bCanRegenAfterSprint = TimeSinceSprintStopped >= StaminaRegenDelaySeconds;
			const bool bCanRegenAfterAttack = TimeSinceLastAttack >= StaminaRegenDelayAfterAttackSeconds;
			if (bCanRegenAfterSprint && bCanRegenAfterAttack)
			{
				const float Regen = StaminaRegenPerSecond * DeltaTime;
				AbilitySystemComponent->ApplyModToAttributeUnsafe(
					BasicAttributeSet->GetStaminaAttribute(),
					EGameplayModOp::Additive,
					Regen
				);
			}
		}

		// ApplyModToAttributeUnsafe nie wywołuje clampów z AttributeSet – pilnujemy [0, MaxStamina] ręcznie
		const float Clamped = FMath::Clamp(BasicAttributeSet->GetStamina(), 0.f, BasicAttributeSet->GetMaxStamina());
		BasicAttributeSet->SetStamina(Clamped);
	}
}

// Called to bind functionality to input
void AAnthillCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAnthillCharacterBase::Jump()
{
	// Staminę zabieramy tylko gdy postać faktycznie może skoczyć (np. stoi na ziemi)
	if (!CanJump())
	{
		return;
	}
	if (JumpStaminaCost > 0.f && !ConsumeStamina(JumpStaminaCost))
	{
		return;
	}
	Super::Jump();
}

void AAnthillCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}


void AAnthillCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GrantAbilities(StartingAbilities);
	}

	if (HasAuthority())
	{
		// Jedna kopia stanu z zapisu w GI — tylko pionek gracza może ją pobrać (moby nie zjadają transformu).
		if (Cast<APlayerController>(NewController))
		{
			ApplyInitialStatsAndLoadRestore();
		}
		else
		{
			ApplyDefaultSpawnStatsOnly();
		}
	}
}

UAbilitySystemComponent* AAnthillCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAnthillCharacterBase::ApplyDamageToSelf(float DamageAmount)
{
	if (!HasAuthority() || !BasicAttributeSet || DamageAmount <= 0.f)
	{
		return;
	}

	// Modyfikujemy tylko CurrentValue (get/set + clamp), żeby BaseValue pozostawał stały (MaxHealth).
	const float MaxH = BasicAttributeSet->GetMaxHealth();
	float H = BasicAttributeSet->GetHealth();
	H = FMath::Clamp(H - DamageAmount, 0.f, MaxH);
	BasicAttributeSet->SetHealth(H);
}

void AAnthillCharacterBase::Respawn()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	UWorld* World = GetWorld();
	if (!PC || !World)
	{
		return;
	}
	AGameModeBase* GM = World->GetAuthGameMode();
	if (GM)
	{
		GM->RestartPlayer(PC);
	}
}

void AAnthillCharacterBase::ReviveAtPlayerStart()
{
	if (!HasAuthority() || !BasicAttributeSet)
	{
		return;
	}

	// Teleport na Player Start
	APlayerController* PC = Cast<APlayerController>(GetController());
	UWorld* World = GetWorld();
	if (PC && World)
	{
		AGameModeBase* GM = World->GetAuthGameMode();
		if (GM)
		{
			AActor* StartSpot = GM->FindPlayerStart(PC);
			if (StartSpot)
			{
				SetActorLocation(StartSpot->GetActorLocation());
				SetActorRotation(StartSpot->GetActorRotation());
				if (UCharacterMovementComponent* Move = GetCharacterMovement())
				{
					Move->Velocity = FVector::Zero();
				}
			}
		}
		else
		{
			// Brak Game Mode – szukaj dowolnego PlayerStart w świecie
			TArray<AActor*> Starts;
			UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
			if (Starts.Num() > 0)
			{
				SetActorLocation(Starts[0]->GetActorLocation());
				SetActorRotation(Starts[0]->GetActorRotation());
				if (UCharacterMovementComponent* Move = GetCharacterMovement())
				{
					Move->Velocity = FVector::Zero();
				}
			}
		}
	}

	// HP i stamina na 100%
	const float MaxH = BasicAttributeSet->GetMaxHealth();
	const float MaxS = BasicAttributeSet->GetMaxStamina();
	BasicAttributeSet->SetHealth(MaxH);
	BasicAttributeSet->SetStamina(MaxS);

	// Włączenie inputu (gdy było wyłączone przy śmierci)
	if (PC)
	{
		PC->EnableInput(PC);
	}
}

void AAnthillCharacterBase::Heal(float Amount)
{
	if (!HasAuthority() || !BasicAttributeSet || Amount <= 0.f) return;
	const float MaxH = BasicAttributeSet->GetMaxHealth();
	float H = FMath::Clamp(BasicAttributeSet->GetHealth() + Amount, 0.f, MaxH);
	BasicAttributeSet->SetHealth(H);
}

void AAnthillCharacterBase::AddMaxHealth(float Amount)
{
	if (!HasAuthority() || !BasicAttributeSet || Amount <= 0.f) return;
	const float NewMax = BasicAttributeSet->GetMaxHealth() + Amount;
	BasicAttributeSet->SetMaxHealth(NewMax);
	float H = FMath::Min(BasicAttributeSet->GetHealth() + Amount, NewMax);
	BasicAttributeSet->SetHealth(H);
}

void AAnthillCharacterBase::AddMaxStamina(float Amount)
{
	if (!HasAuthority() || !BasicAttributeSet || Amount <= 0.f) return;
	const float NewMax = BasicAttributeSet->GetMaxStamina() + Amount;
	BasicAttributeSet->SetMaxStamina(NewMax);
	float S = FMath::Min(BasicAttributeSet->GetStamina() + Amount, NewMax);
	BasicAttributeSet->SetStamina(S);
}

void AAnthillCharacterBase::SetAttackBuff(float Multiplier, float DurationSeconds)
{
	if (!GetWorld() || DurationSeconds <= 0.f) return;
	AttackBuffMultiplier = FMath::Max(1.f, Multiplier);
	AttackBuffEndTime = GetWorld()->GetTimeSeconds() + DurationSeconds;
}

float AAnthillCharacterBase::GetAttackDamageMultiplier() const
{
	const UWorld* World = GetWorld();
	if (!World || World->GetTimeSeconds() >= AttackBuffEndTime)
		return 1.f;
	return AttackBuffMultiplier;
}

// --- Pasek przedmiotów (EQ) ---
bool AAnthillCharacterBase::AddItemToInventory(EPickupType Type, float Value1, float Value2)
{
	if (!HasAuthority() || InventorySlots.Num() != InventorySlotCount) return false;

	const int32 MaxStack = FMath::Max(1, MaxInventoryStackPerSlot);

	FInventorySlot Candidate;
	Candidate.bValid = true;
	Candidate.Type = Type;
	Candidate.Value1 = Value1;
	Candidate.Value2 = Value2;

	// Najpierw dokładaj do istniejącego stosu (ten sam typ i parametry).
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (Slot.bValid && InventorySlotsStackTogether(Slot, Candidate))
		{
			if (Slot.StackCount < MaxStack)
			{
				Slot.StackCount++;
				return true;
			}
		}
	}

	// Pełny stos lub brak dopasowania — pierwszy wolny slot.
	for (FInventorySlot& Slot : InventorySlots)
	{
		if (!Slot.bValid)
		{
			Slot = Candidate;
			Slot.StackCount = 1;
			return true;
		}
	}
	return false;
}

bool AAnthillCharacterBase::UseInventorySlot(int32 SlotIndex)
{
	if (!HasAuthority() || SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return false;
	FInventorySlot& Slot = InventorySlots[SlotIndex];
	if (!Slot.bValid || !BasicAttributeSet) return false;

	if (const UWorld* World = GetWorld())
	{
		const float Now = World->GetTimeSeconds();
		if (Now - LastInventoryUseTime < InventoryUseMinIntervalSeconds)
		{
			return false;
		}
	}

	if (Slot.StackCount < 1)
	{
		Slot.StackCount = 1;
	}

	switch (Slot.Type)
	{
	case EPickupType::Heal:
		Heal(Slot.Value1);
		break;
	case EPickupType::MaxStamina:
		AddMaxStamina(Slot.Value1);
		break;
	case EPickupType::MaxHealth:
		AddMaxHealth(Slot.Value1);
		break;
	case EPickupType::AttackBuff:
		SetAttackBuff(Slot.Value1, Slot.Value2);
		break;
	default:
		break;
	}

	Slot.StackCount = FMath::Max(0, Slot.StackCount - 1);
	if (Slot.StackCount <= 0)
	{
		Slot.bValid = false;
		Slot.Value1 = Slot.Value2 = 0.f;
		Slot.StackCount = 0;
	}
	if (const UWorld* World = GetWorld())
	{
		LastInventoryUseTime = World->GetTimeSeconds();
	}
	return true;
}

FInventorySlot AAnthillCharacterBase::GetInventorySlot(int32 SlotIndex) const
{
	FInventorySlot Empty;
	Empty.bValid = false;
	if (SlotIndex < 0 || SlotIndex >= InventorySlots.Num()) return Empty;
	return InventorySlots[SlotIndex];
}

void AAnthillCharacterBase::SetSelectedInventorySlotIndex(int32 Index)
{
	SelectedInventorySlotIndex = FMath::Clamp(Index, 0, InventorySlotCount - 1);
}

bool AAnthillCharacterBase::UseSelectedInventorySlot()
{
	return UseInventorySlot(SelectedInventorySlotIndex);
}

TArray<FGameplayAbilitySpecHandle> AAnthillCharacterBase::GrantAbilities(
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant)
{
	if (!AbilitySystemComponent)
	{
		return TArray<FGameplayAbilitySpecHandle>();
	}
	
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	
	for (TSubclassOf<UGameplayAbility> Ability : AbilitiesToGrant)
	{
		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(
			Ability, 1, -1, this));
		AbilityHandles.Add(SpecHandle);
	}
	
	return AbilityHandles;
}

void AAnthillCharacterBase::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	
	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}
}

void AAnthillCharacterBase::OnRep_InventorySlots()
{
	// Można tu wywołać delegat/event dla UI, żeby odświeżyć pasek.
}

void AAnthillCharacterBase::StartSprint()
{
	bIsSprinting = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = SprintSpeed;
	}
}

void AAnthillCharacterBase::StopSprint()
{
	bIsSprinting = false;
	TimeSinceSprintStopped = 0.f;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeed;
	}
}

float AAnthillCharacterBase::GetHealth() const
{
	return BasicAttributeSet ? BasicAttributeSet->GetHealth() : 0.f;
}

float AAnthillCharacterBase::GetMaxHealth() const
{
	return BasicAttributeSet ? BasicAttributeSet->GetMaxHealth() : 0.f;
}

float AAnthillCharacterBase::GetStamina() const
{
	return BasicAttributeSet ? BasicAttributeSet->GetStamina() : 0.f;
}

float AAnthillCharacterBase::GetMaxStamina() const
{
	return BasicAttributeSet ? BasicAttributeSet->GetMaxStamina() : 0.f;
}

void AAnthillCharacterBase::NotifyAttackPerformed()
{
	TimeSinceLastAttack = 0.f;
	TimeSinceLastAttackTrigger = 0.f;
}

bool AAnthillCharacterBase::CanPerformAttack() const
{
	return TimeSinceLastAttackTrigger >= AttackCooldownSeconds;
}

bool AAnthillCharacterBase::ConsumeStamina(float Amount)
{
	if (!HasAuthority() || !AbilitySystemComponent || !BasicAttributeSet || Amount <= 0.f)
	{
		return false;
	}

	// Uwzględniamy tylko staminę w zakresie [0, Max] (na wypadek wcześniejszego overflowu)
	const float MaxSt = BasicAttributeSet->GetMaxStamina();
	float Current = BasicAttributeSet->GetStamina();
	if (Current > MaxSt)
	{
		BasicAttributeSet->SetStamina(MaxSt);
		Current = MaxSt;
	}
	if (Current < Amount)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		BasicAttributeSet->GetStaminaAttribute(),
		EGameplayModOp::Additive,
		-Amount
	);
	// Clamp po odjęciu (ApplyModToAttributeUnsafe nie wywołuje PreAttributeChange)
	BasicAttributeSet->SetStamina(FMath::Clamp(BasicAttributeSet->GetStamina(), 0.f, MaxSt));
	return true;
}


