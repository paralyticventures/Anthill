// Fill out your copyright notice in the Description page of Project Settings.

#include "AnthillPlayerController.h"
#include "Anthill/UI/HealthStaminaWidget.h"
#include "Blueprint/UserWidget.h"

AAnthillPlayerController::AAnthillPlayerController()
{
	HealthStaminaWidgetClass = nullptr;
	HealthStaminaWidget = nullptr;
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
}
