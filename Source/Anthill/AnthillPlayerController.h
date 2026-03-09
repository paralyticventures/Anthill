// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AnthillPlayerController.generated.h"

class UHealthStaminaWidget;

/**
 * Player Controller tworzący widget Health/Stamina i dodający go do ekranu.
 * W edytorze: ustaw Widget Class na swój Blueprint widget (np. WBP_HealthStamina).
 */
UCLASS()
class ANTHILL_API AAnthillPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAnthillPlayerController();

	/** Klasa widgetu do wyświetlenia (ustaw w Blueprint na WBP_HealthStamina). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthStaminaWidgetClass;

	/** Utworzona instancja widgetu (np. do ukrywania/pokazywania). */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> HealthStaminaWidget;

protected:
	virtual void BeginPlay() override;
};
