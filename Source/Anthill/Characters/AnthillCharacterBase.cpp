// Fill out your copyright notice in the Description page of Project Settings.


#include "AnthillCharacterBase.h"

#include "Anthill/GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
}

// Called when the game starts or when spawned
void AAnthillCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAnthillCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	if (H <= 0.f)
	{
		OnDeath();
	}
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


