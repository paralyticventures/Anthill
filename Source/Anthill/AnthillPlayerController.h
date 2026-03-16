// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AnthillPlayerController.generated.h"

class UHealthStaminaWidget;
class UInventoryBarWidget;

/**
 * Player Controller tworzący widgety Health/Stamina oraz pasek przedmiotów (EQ).
 * W edytorze: ustaw Widget Class na swoje Blueprinty (np. WBP_HealthStamina, WBP_InventoryBar).
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

	/** Klasa widgetu paska przedmiotów (ustaw w Blueprint na WBP_InventoryBar). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> InventoryBarWidgetClass;

	/** Utworzona instancja paska przedmiotów. */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> InventoryBarWidget;

	/** Używa przedmiotu z aktualnie wybranego slotu (0–7). Podłącz do klawisza np. R w Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseSelectedInventoryItem();

	/** Ustawia wybrany slot (0–7). Podłącz klawisze 1–8 w Blueprint: Select Inventory Slot(0) … Select Inventory Slot(7). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectInventorySlot(int32 SlotIndex);

protected:
	virtual void BeginPlay() override;
};
