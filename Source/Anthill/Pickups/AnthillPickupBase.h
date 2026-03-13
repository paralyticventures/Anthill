// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnthillPickupBase.generated.h"

class USphereComponent;
class AAnthillCharacterBase;

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	Heal           UMETA(DisplayName = "Pasta odnawiająca HP"),
	MaxStamina     UMETA(DisplayName = "Pasta zwiększająca max Staminę"),
	MaxHealth      UMETA(DisplayName = "Pasta zwiększająca max HP"),
	AttackBuff     UMETA(DisplayName = "Pasta dająca buff do ataku")
};

/**
 * Bazowy pickup (np. pasta do zębów). Ustaw Typ i wartości w Class Defaults lub w Blueprintie.
 */
UCLASS(Abstract)
class ANTHILL_API AAnthillPickupBase : public AActor
{
	GENERATED_BODY()

public:
	AAnthillPickupBase();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	USphereComponent* CollisionSphere;

	/** true = przedmiot trafia na pasek przedmiotów (EQ), false = użycie natychmiastowe (jak dotąd). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	bool bAddToInventory = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	EPickupType PickupType = EPickupType::Heal;

	/** Dla Heal: ile HP przywrócić. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "PickupType == EPickupType::Heal", ClampMin = "0"))
	float HealAmount = 25.f;

	/** Dla MaxStamina: o ile zwiększyć max staminę (i obecną). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "PickupType == EPickupType::MaxStamina", ClampMin = "0"))
	float MaxStaminaAmount = 20.f;

	/** Dla MaxHealth: o ile zwiększyć max HP (i obecne). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "PickupType == EPickupType::MaxHealth", ClampMin = "0"))
	float MaxHealthAmount = 25.f;

	/** Dla AttackBuff: mnożnik obrażeń (np. 1.5 = +50%%). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "PickupType == EPickupType::AttackBuff", ClampMin = "1.0"))
	float AttackBuffMultiplier = 1.5f;

	/** Dla AttackBuff: czas trwania buffa w sekundach. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (EditCondition = "PickupType == EPickupType::AttackBuff", ClampMin = "0.1"))
	float AttackBuffDurationSeconds = 10.f;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;
};
