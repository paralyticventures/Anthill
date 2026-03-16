// Fill out your copyright notice in the Description page of Project Settings.

#include "AnthillPlayerController.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Anthill/UI/HealthStaminaWidget.h"
#include "Anthill/UI/InventoryBarWidget.h"
#include "Blueprint/UserWidget.h"

AAnthillPlayerController::AAnthillPlayerController()
{
	HealthStaminaWidgetClass = nullptr;
	HealthStaminaWidget = nullptr;
	InventoryBarWidgetClass = nullptr;
	InventoryBarWidget = nullptr;
}

void AAnthillPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HealthStaminaWidgetClass)
	{
		HealthStaminaWidget = CreateWidget<UUserWidget>(this, HealthStaminaWidgetClass);
		if (HealthStaminaWidget)
		{
			HealthStaminaWidget->AddToViewport();
		}
	}

	if (InventoryBarWidgetClass)
	{
		InventoryBarWidget = CreateWidget<UUserWidget>(this, InventoryBarWidgetClass);
		if (InventoryBarWidget)
		{
			InventoryBarWidget->AddToViewport();
		}
	}
}

void AAnthillPlayerController::UseSelectedInventoryItem()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (AAnthillCharacterBase* AnthillChar = Cast<AAnthillCharacterBase>(ControlledPawn))
		{
			AnthillChar->UseSelectedInventorySlot();
		}
	}
}

void AAnthillPlayerController::SelectInventorySlot(int32 SlotIndex)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (AAnthillCharacterBase* AnthillChar = Cast<AAnthillCharacterBase>(ControlledPawn))
		{
			AnthillChar->SetSelectedInventorySlotIndex(SlotIndex);
		}
	}
}
