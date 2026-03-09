// Fill out your copyright notice in the Description page of Project Settings.


#include "AnthillCharacterBase.h"

#include "Anthill/GameplayAbilitySystem/Attributes/BasicAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
				// Stoi z wciśniętym Shift – traktujemy jak odpoczynek, stamina się odnawia.
				const float Regen = StaminaRegenPerSecond * DeltaTime;
				AbilitySystemComponent->ApplyModToAttributeUnsafe(
					BasicAttributeSet->GetStaminaAttribute(),
					EGameplayModOp::Additive,
					Regen
				);
			}

			if (BasicAttributeSet->GetStamina() <= 0.f)
			{
				StopSprint();
			}
		}
		else
		{
			TimeSinceSprintStopped += DeltaTime;
			if (TimeSinceSprintStopped >= StaminaRegenDelaySeconds)
			{
				const float Regen = StaminaRegenPerSecond * DeltaTime;
				AbilitySystemComponent->ApplyModToAttributeUnsafe(
					BasicAttributeSet->GetStaminaAttribute(),
					EGameplayModOp::Additive,
					Regen
				);
			}
		}
	}
}

// Called to bind functionality to input
void AAnthillCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAnthillCharacterBase::Jump()
{
	if (JumpStaminaCost > 0.f && !ConsumeStamina(JumpStaminaCost))
	{
		return; // Za mało staminy – brak skoku
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
	if (!HasAuthority() || !AbilitySystemComponent || !BasicAttributeSet)
	{
		return;
	}

	if (DamageAmount <= 0.f)
	{
		return;
	}

	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		BasicAttributeSet->GetHealthAttribute(),
		EGameplayModOp::Additive,
		-DamageAmount
	);
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

bool AAnthillCharacterBase::ConsumeStamina(float Amount)
{
	if (!HasAuthority() || !AbilitySystemComponent || !BasicAttributeSet || Amount <= 0.f)
	{
		return false;
	}

	const float Current = BasicAttributeSet->GetStamina();
	if (Current < Amount)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		BasicAttributeSet->GetStaminaAttribute(),
		EGameplayModOp::Additive,
		-Amount
	);
	return true;
}


