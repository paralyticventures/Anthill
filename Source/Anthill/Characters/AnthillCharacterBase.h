// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Anthill/Pickups/AnthillPickupBase.h"
#include "AnthillCharacterBase.generated.h"

/** Pojedynczy slot paska przedmiotów (ekwipunku). */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bValid = false;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	EPickupType Type = EPickupType::Heal;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float Value1 = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float Value2 = 0.f;
};

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	/** Wywołaj z Blueprinta po wykonaniu ataku – opóźnia regenerację staminy i włącza cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void NotifyAttackPerformed();

	/** Czy można teraz wykonać atak (false w trakcie cooldownu po poprzednim ataku). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanPerformAttack() const;

	/** Wywoływane gdy Health spadnie do 0 (Blueprint może zareagować: animacja śmierci, wyłączenie sterowania). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat", meta = (DisplayName = "On Death"))
	void OnDeath();

	/** Odrodzenie: wywołaj z Blueprinta (np. po naciśnięciu R na ekranie śmierci). Prosi Game Mode o respawn (nowa postać). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Respawn();

	/** Odrodzenie w miejscu: teleport na Player Start, HP i stamina na 100%%, włączenie inputu. Użyj zamiast Respawn, jeśli RestartPlayer nie działa. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReviveAtPlayerStart();

	// --- Pickupy (pasta itp.) ---
	/** Przywraca HP (clamp do MaxHealth). */
	UFUNCTION(BlueprintCallable, Category = "Pickups")
	void Heal(float Amount);

	/** Zwiększa MaxHealth i opcjonalnie obecne HP. */
	UFUNCTION(BlueprintCallable, Category = "Pickups")
	void AddMaxHealth(float Amount);

	/** Zwiększa MaxStamina i opcjonalnie obecną staminę. */
	UFUNCTION(BlueprintCallable, Category = "Pickups")
	void AddMaxStamina(float Amount);

	/** Buff do obrażeń ataku: mnożnik (np. 1.5 = +50%%), czas trwania w sekundach. */
	UFUNCTION(BlueprintCallable, Category = "Pickups")
	void SetAttackBuff(float Multiplier, float DurationSeconds);

	/** Mnożnik obrażeń ataku (1.0 = brak buffa). Użyj w systemie walki przy liczeniu damage. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pickups")
	float GetAttackDamageMultiplier() const;

	// --- Pasek przedmiotów (EQ) ---
	/** Liczba slotów na pasku (np. 8). */
	static constexpr int32 InventorySlotCount = 8;

	/** Dodaje przedmiot do pierwszego wolnego slotu. Zwraca true, jeśli dodano. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItemToInventory(EPickupType Type, float Value1, float Value2 = 0.f);

	/** Używa przedmiotu ze slotu (np. leczy, daje buff). Zwraca true, jeśli slot był zajęty i użyto. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseInventorySlot(int32 SlotIndex);

	/** Zwraca dane slotu (0..InventorySlotCount-1). Dla pustego slotu bValid = false. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FInventorySlot GetInventorySlot(int32 SlotIndex) const;

	/** Indeks aktualnie wybranego slotu (0..InventorySlotCount-1). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSelectedInventorySlotIndex() const { return SelectedInventorySlotIndex; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSelectedInventorySlotIndex(int32 Index);

	/** Używa przedmiotu z aktualnie wybranego slotu. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseSelectedInventorySlot();
	
	// Array of abilities to grant & remove
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint")
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Sprint")
	float SprintSpeed = 750.f;

	/** Początkowe / bazowe maks. HP (BasicAttributeSet → MaxHealth). Ustaw w BP jak staminę. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Health", meta = (ClampMin = "0"))
	float DefaultMaxHealth = 100.f;

	/** Początkowe HP jako ułamek MaxHealth (1 = pełne, 0.5 = połowa). Zaclamowane po starcie. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Health", meta = (ClampMin = "0", ClampMax = "1"))
	float StartingHealthPercent = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaDrainPerSecond = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaRegenPerSecond = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina")
	float StaminaRegenDelaySeconds = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina", meta = (ClampMin = "0"))
	float JumpStaminaCost = 15.f;

	/** Czas (s) po ataku, po którym stamina znowu się regeneruje. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Stamina", meta = (ClampMin = "0"))
	float StaminaRegenDelayAfterAttackSeconds = 1.5f;

	/** Cooldown ataku (s) – przez ten czas kolejne kliknięcia nie wykonają ataku ani nie zabiorą staminy. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0"))
	float AttackCooldownSeconds = 0.6f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|Sprint")
	bool bIsSprinting = false;

	float TimeSinceSprintStopped = 0.f;
	float TimeSinceLastAttack = 9999.f;
	float TimeSinceLastAttackTrigger = 9999.f;

	float AttackBuffMultiplier = 1.f;
	float AttackBuffEndTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 SelectedInventorySlotIndex = 0;

	UFUNCTION()
	void OnRep_InventorySlots();
};
