#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Anthill/AnthillInventoryTypes.h"
#include "AnthillSaveGame.generated.h"

/** Minimalne dane zapisu (rozszerz o stan gracza / ekwipunek w miarę potrzeb). */
UCLASS(BlueprintType)
class ANTHILL_API UAnthillSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Krótki opis do UI (np. „Slot 1”). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString SlotDisplayName;

	/**
	 * Identyfikator mapy do OpenLevel — zwykle krótka nazwa z UGameplayStatics::GetCurrentLevelName
	 * (np. Lvl_FirstPerson). Możesz też zapisać pełną ścieżkę /Game/... — OpenLevelByPath obsłuży oba warianty.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FString MapPackagePath;

	/** Czas gry (sekundy) — do wyświetlenia w Load Menu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float TotalPlayTimeSeconds = 0.f;

	/** Czas utworzenia zapisu. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FDateTime SavedAt = FDateTime::UtcNow();

	/** Jeśli true, po wczytaniu mapy postać zostanie ustawiona w zapisanej pozycji (nie przy Player Start). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bHasPlayerTransform = false;

	/** Pozycja i rotacja postaci w momencie zapisu (świat gry). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	FTransform PlayerTransform;

	/** Stan GAS + ekwipunek (wypełniane przy zapisie z AnthillCharacterBase). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	bool bHasCharacterState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SavedHealth = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SavedMaxHealth = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SavedStamina = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	float SavedMaxStamina = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FInventorySlot> SavedInventorySlots;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	int32 SavedSelectedInventorySlotIndex = 0;

	/**
	 * Klucze zebranych pickupów (format: NazwaMapy|ActorName lub NazwaMapy|SavePickupIdOverride).
	 * Po Load te aktory nie respawnują się na mapie.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
	TArray<FString> CollectedPickupKeys;
};
