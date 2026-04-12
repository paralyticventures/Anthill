#pragma once

#include "CoreMinimal.h"
#include "Anthill/Pickups/AnthillPickupBase.h"
#include "AnthillInventoryTypes.generated.h"

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

	/** Ilość w stosie (≥1 gdy bValid). Przy użyciu jednej sztuki maleje o 1. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 StackCount = 0;
};
