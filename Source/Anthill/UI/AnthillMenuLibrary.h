#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnthillMenuLibrary.generated.h"

class UAnthillSaveGame;

USTRUCT(BlueprintType)
struct FAnthillSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	bool bHasSave = false;

	/** Nazwa wyświetlana zapisu (SlotDisplayName w save). */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString SlotLabel;

	/** Data/czas zapisu (UTC), sformatowane — do jednej linii pod nazwą. */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString SavedAtText;

	/** Czas gry w zapisie (sekundy). */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	float PlayTimeSeconds = 0.f;

	/** Krótka nazwa mapy z zapisu. */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString MapNameShort;

	/** Jedna linijka ze wszystkim (kompatybilność wstecz). */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	FString DescriptionText;
};

/**
 * Funkcje wywoływane z widgetów Main Menu / Load Menu (Blueprint).
 */
UCLASS()
class ANTHILL_API UAnthillMenuLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Zamyka aplikację (działa w buildach, w PIE kończy sesję). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Menu", meta = (WorldContext = "WorldContextObject"))
	static void QuitGame(UObject* WorldContextObject);

	/** Otwiera mapę z GameInstance (Main Menu). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Menu", meta = (WorldContext = "WorldContextObject"))
	static void OpenMainMenuLevel(UObject* WorldContextObject);

	/** Otwiera mapę Load Menu z GameInstance. */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Menu", meta = (WorldContext = "WorldContextObject"))
	static void OpenLoadMenuLevel(UObject* WorldContextObject);

	/** Otwiera mapę gry (New Game) z GameInstance — patrz UAnthillGameInstance::GameLevel. */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Menu", meta = (WorldContext = "WorldContextObject"))
	static void OpenGameLevel(UObject* WorldContextObject);

	/** Otwiera dowolną mapę po pełnej ścieżce assetu (np. z zapisu). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Menu", meta = (WorldContext = "WorldContextObject"))
	static void OpenLevelByPath(UObject* WorldContextObject, const FString& MapPackagePath);

	/** Prefiks nazwy pliku zapisu: AnthillSave0 … AnthillSave2. */
	UFUNCTION(BlueprintPure, Category = "Anthill|Save")
	static FString GetSaveSlotName(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Anthill|Save")
	static bool DoesSaveExistInSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Anthill|Save", meta = (WorldContext = "WorldContextObject"))
	static FAnthillSaveSlotInfo GetSaveSlotInfo(UObject* WorldContextObject, int32 SlotIndex);

	/** Zapisuje bieżącą mapę do slotu (UserIndex 0). Wywołuj z poziomu gry (np. z menu pauzy). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Save", meta = (WorldContext = "WorldContextObject"))
	static bool SaveCurrentGameToSlot(UObject* WorldContextObject, int32 SlotIndex, const FString& DisplayName);

	/** Ładuje mapę z podanego slotu (jeśli istnieje zapis). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Save", meta = (WorldContext = "WorldContextObject"))
	static bool LoadGameFromSlotAndOpenMap(UObject* WorldContextObject, int32 SlotIndex);

	/** Usuwa zapis w slocie. */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Save")
	static bool DeleteSaveInSlot(int32 SlotIndex);

	/** „Continue”: ładuje pierwszy slot z zapisem (0, potem 1, 2). */
	UFUNCTION(BlueprintCallable, Category = "Anthill|Save", meta = (WorldContext = "WorldContextObject"))
	static bool ContinueLastSave(UObject* WorldContextObject);

private:
	static const int32 SaveUserIndex;
	static FString SlotNameFromIndex(int32 SlotIndex);
};
