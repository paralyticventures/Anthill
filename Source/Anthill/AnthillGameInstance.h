#pragma once

#include "CoreMinimal.h"
#include "Anthill/AnthillInventoryTypes.h"
#include "Engine/GameInstance.h"
#include "Math/Transform.h"
#include "AnthillGameInstance.generated.h"

/** Kopia danych z UAnthillSaveGame przed OpenLevel — konsumowana przez postać po wejściu na mapę. */
struct FAnthillPendingLoadData
{
	bool bHasPending = false;
	bool bHasTransform = false;
	FTransform Transform;
	bool bHasCharacterState = false;
	float Health = 0.f;
	float MaxHealth = 100.f;
	float Stamina = 0.f;
	float MaxStamina = 100.f;
	TArray<FInventorySlot> InventorySlots;
	int32 SelectedInventorySlotIndex = 0;
};

class UAnthillSaveGame;

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

	/** Wywołaj po wczytaniu UAnthillSaveGame, przed OpenLevel. */
	void SetPendingLoadFromSaveGame(const UAnthillSaveGame* Save);
	void ClearPendingLoad();
	bool ConsumePendingLoad(FAnthillPendingLoadData& OutData);

private:
	FAnthillPendingLoadData PendingLoad;
};
