// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AnthillCharacterBase.generated.h"

UCLASS()
class ANTHILL_API AAnthillCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAnthillCharacterBase();
	
	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributeSet;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Jump() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Damage
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ApplyDamageToSelf(float DamageAmount);

	// Sprint / Stamina
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	// Health
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetHealth() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetMaxHealth() const;

	// Stamina
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetStamina() const;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetMaxStamina() const;

	/** Zużywa staminę (np. przy ataku). Zwraca true, jeśli było wystarczająco staminy i odjęto koszt. */
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ConsumeStamina(float Amount);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint")
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint")
	float SprintSpeed = 750.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaDrainPerSecond = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaRegenPerSecond = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaRegenDelaySeconds = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina", meta = (ClampMin = "0"))
	float JumpStaminaCost = 15.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint")
	bool bIsSprinting = false;

	float TimeSinceSprintStopped = 0.f;
};
