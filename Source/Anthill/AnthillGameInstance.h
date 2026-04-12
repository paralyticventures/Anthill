#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AnthillGameInstance.generated.h"

/** Globalny stan + ścieżki map. Domyślne wartości w konstruktorze (AnthillGameInstance.cpp). */
UCLASS()
class ANTHILL_API UAnthillGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UAnthillGameInstance();

	/** Main menu (logo, New Game, Load Game, Exit). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anthill|Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	FSoftObjectPath MainMenuLevel;

	/** Ekran wyboru / zarządzania zapisami (osobna mapa). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anthill|Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	FSoftObjectPath LoadMenuLevel;

	/**
	 * Poziom rozgrywki po „New Game” oraz domyślny przy zapisie, gdy nie da się odczytać aktualnej mapy.
	 * Zmień w konstruktorze, gdy zmienisz mapę startową.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anthill|Maps", meta = (AllowedClasses = "/Script/Engine.World"))
	FSoftObjectPath GameLevel;

	/** Skumulowany czas gry (sekundy) — aktualizowany na mapie gry, zapisywany w slocie. */
	UPROPERTY(BlueprintReadOnly, Category = "Anthill|Save")
	float RunningPlayTimeSeconds = 0.f;

	/** Wywołaj przy „New Game” (czas od zera). */
	void ResetSessionPlayTime();

	/** Po wczytaniu zapisu — kontynuacja licznika z pliku. */
	void SetSessionPlayTimeFromSave(float TotalSecondsFromSave);

	/** Wołane z PlayerController::Tick — tylko mapa gry, bez pauzy. */
	void AddPlayTimeSeconds(float DeltaSeconds, const class UWorld* World);

	float GetRunningPlayTimeSeconds() const { return RunningPlayTimeSeconds; }

	bool IsPlaytimeGameplayMap(const class UWorld* World) const;
};
