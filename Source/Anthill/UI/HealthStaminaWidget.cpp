// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthStaminaWidget.h"
#include "Anthill/Characters/AnthillCharacterBase.h"
#include "Kismet/GameplayStatics.h"

void UHealthStaminaWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshFromCharacter();
}

void UHealthStaminaWidget::RefreshFromCharacter()
{
	AAnthillCharacterBase* Character = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		Character = Cast<AAnthillCharacterBase>(PC->GetPawn());
	}

	if (!Character)
	{
		return;
	}

	MaxHealth = Character->GetMaxHealth();
	MaxStamina = Character->GetMaxStamina();
	Health = Character->GetHealth();
	Stamina = Character->GetStamina();

	HealthPercent = MaxHealth > 0.f ? (Health / MaxHealth) : 0.f;
	StaminaPercent = MaxStamina > 0.f ? (Stamina / MaxStamina) : 0.f;
}
