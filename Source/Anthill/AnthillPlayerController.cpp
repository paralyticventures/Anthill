// Fill out your copyright notice in the Description page of Project Settings.

#include "AnthillPlayerController.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Anthill/UI/HealthStaminaWidget.h"
#include "Anthill/UI/InventoryBarWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"

AAnthillPlayerController::AAnthillPlayerController()
{
	HealthStaminaWidgetClass = nullptr;
	HealthStaminaWidget = nullptr;
	InventoryBarWidgetClass = nullptr;
	InventoryBarWidget = nullptr;
	PauseMenuWidgetClass = nullptr;
	PauseMenuWidget = nullptr;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AAnthillPlayerController::ApplyGameInputMode()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);
}

void AAnthillPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyGameInputMode();

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

void AAnthillPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ApplyGameInputMode();
}

void AAnthillPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AAnthillPlayerController::TogglePauseMenu);
	}
}

void AAnthillPlayerController::TogglePauseMenu()
{
	if (!PauseMenuWidgetClass)
	{
		return;
	}
	if (bPauseMenuOpen)
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void AAnthillPlayerController::ShowPauseMenu()
{
	if (!PauseMenuWidgetClass || bPauseMenuOpen)
	{
		return;
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuWidgetClass);
	}
	if (!PauseMenuWidget)
	{
		return;
	}

	PauseMenuWidget->AddToViewport(100);
	bPauseMenuOpen = true;

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, true);
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AAnthillPlayerController::HidePauseMenu()
{
	if (!bPauseMenuOpen)
	{
		return;
	}

	bPauseMenuOpen = false;

	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
	}

	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	ApplyGameInputMode();
}

void AAnthillPlayerController::ClosePauseMenu()
{
	HidePauseMenu();
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
