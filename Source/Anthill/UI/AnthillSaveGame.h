#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
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
};
