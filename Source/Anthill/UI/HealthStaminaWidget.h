// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthStaminaWidget.generated.h"

class AAnthillCharacterBase;

/**
 * Widget wyświetlający Health i Stamina postaci.
 * W Blueprint: utwórz Widget Blueprint dziedziczący z tej klasy,
 * dodaj Progress Bar / Text i powiąż z HealthPercent, StaminaPercent lub wartościami.
 */
UCLASS()
class ANTHILL_API UHealthStaminaWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Odświeża wartości z aktualnej postaci gracza (wywołaj z Blueprint lub używaj NativeTick). */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshFromCharacter();

	/** Procent zdrowia 0–1 (do Progress Bar). */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float HealthPercent = 1.f;

	/** Procent staminy 0–1 (do Progress Bar). */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float StaminaPercent = 1.f;

	/** Aktualne zdrowie (np. do Text). */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Health = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float Stamina = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	float MaxStamina = 100.f;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
