#include "AnthillMenuLibrary.h"

#include "../AnthillGameInstance.h"
#include "AnthillSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

const int32 UAnthillMenuLibrary::SaveUserIndex = 0;

FString UAnthillMenuLibrary::SlotNameFromIndex(int32 SlotIndex)
{
	return FString::Printf(TEXT("AnthillSave%d"), FMath::Clamp(SlotIndex, 0, 99));
}

FString UAnthillMenuLibrary::GetSaveSlotName(int32 SlotIndex)
{
	return SlotNameFromIndex(SlotIndex);
}

void UAnthillMenuLibrary::QuitGame(UObject* WorldContextObject)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	UKismetSystemLibrary::QuitGame(WorldContextObject, PC, EQuitPreference::Quit, false);
}

static void OpenLevelFromSoftPath(UObject* WorldContextObject, const FSoftObjectPath& Path)
{
	if (!Path.IsValid())
	{
		return;
	}
	// OpenLevel przyjmuje zwykle krótką nazwę assetu mapy.
	const FName LevelName(*Path.GetAssetName());
	UGameplayStatics::OpenLevel(WorldContextObject, LevelName, true, FString());
}

void UAnthillMenuLibrary::OpenMainMenuLevel(UObject* WorldContextObject)
{
	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		OpenLevelFromSoftPath(WorldContextObject, GI->MainMenuLevel);
	}
}

void UAnthillMenuLibrary::OpenLoadMenuLevel(UObject* WorldContextObject)
{
	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		OpenLevelFromSoftPath(WorldContextObject, GI->LoadMenuLevel);
	}
}

void UAnthillMenuLibrary::OpenGameLevel(UObject* WorldContextObject)
{
	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		GI->ResetSessionPlayTime();
		OpenLevelFromSoftPath(WorldContextObject, GI->GameLevel);
	}
}

void UAnthillMenuLibrary::OpenLevelByPath(UObject* WorldContextObject, const FString& MapPackagePath)
{
	if (MapPackagePath.IsEmpty())
	{
		return;
	}
	// Pełna ścieżka assetu (/Game/...)
	if (MapPackagePath.StartsWith(TEXT("/")))
	{
		const FSoftObjectPath Path(MapPackagePath);
		if (Path.IsValid())
		{
			OpenLevelFromSoftPath(WorldContextObject, Path);
		}
		return;
	}
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*MapPackagePath), true, FString());
}

bool UAnthillMenuLibrary::DoesSaveExistInSlot(int32 SlotIndex)
{
	return UGameplayStatics::DoesSaveGameExist(SlotNameFromIndex(SlotIndex), SaveUserIndex);
}

FAnthillSaveSlotInfo UAnthillMenuLibrary::GetSaveSlotInfo(UObject* WorldContextObject, int32 SlotIndex)
{
	FAnthillSaveSlotInfo Info;
	const FString Name = SlotNameFromIndex(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(Name, SaveUserIndex))
	{
		return Info;
	}

	if (UAnthillSaveGame* Loaded = Cast<UAnthillSaveGame>(UGameplayStatics::LoadGameFromSlot(Name, SaveUserIndex)))
	{
		Info.bHasSave = true;
		Info.SlotLabel = Loaded->SlotDisplayName;
		Info.PlayTimeSeconds = Loaded->TotalPlayTimeSeconds;
		Info.MapNameShort = Loaded->MapPackagePath;
		Info.SavedAtText = Loaded->SavedAt.ToString(TEXT("%Y-%m-%d  %H:%M"));
		Info.DescriptionText = FString::Printf(TEXT("%.0f s"), Loaded->TotalPlayTimeSeconds);
	}
	return Info;
}

bool UAnthillMenuLibrary::SaveCurrentGameToSlot(UObject* WorldContextObject, int32 SlotIndex, const FString& DisplayName)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	UAnthillSaveGame* SaveGameInstance = Cast<UAnthillSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAnthillSaveGame::StaticClass()));
	if (!SaveGameInstance)
	{
		return false;
	}

	SaveGameInstance->SlotDisplayName = DisplayName.IsEmpty()
		? FString::Printf(TEXT("Slot %d"), SlotIndex)
		: DisplayName;

	SaveGameInstance->MapPackagePath = UGameplayStatics::GetCurrentLevelName(World);
	if (SaveGameInstance->MapPackagePath.IsEmpty())
	{
		if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
		{
			if (GI->GameLevel.IsValid())
			{
				SaveGameInstance->MapPackagePath = GI->GameLevel.GetAssetName();
			}
		}
	}

	SaveGameInstance->SavedAt = FDateTime::UtcNow();
	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		SaveGameInstance->TotalPlayTimeSeconds = GI->GetRunningPlayTimeSeconds();
	}

	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotNameFromIndex(SlotIndex), SaveUserIndex);
}

bool UAnthillMenuLibrary::LoadGameFromSlotAndOpenMap(UObject* WorldContextObject, int32 SlotIndex)
{
	const FString Name = SlotNameFromIndex(SlotIndex);
	if (!UGameplayStatics::DoesSaveGameExist(Name, SaveUserIndex))
	{
		return false;
	}

	UAnthillSaveGame* Loaded = Cast<UAnthillSaveGame>(UGameplayStatics::LoadGameFromSlot(Name, SaveUserIndex));
	if (!Loaded || Loaded->MapPackagePath.IsEmpty())
	{
		return false;
	}

	if (UAnthillGameInstance* GI = Cast<UAnthillGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		GI->SetSessionPlayTimeFromSave(Loaded->TotalPlayTimeSeconds);
	}

	OpenLevelByPath(WorldContextObject, Loaded->MapPackagePath);
	return true;
}

bool UAnthillMenuLibrary::DeleteSaveInSlot(int32 SlotIndex)
{
	return UGameplayStatics::DeleteGameInSlot(SlotNameFromIndex(SlotIndex), SaveUserIndex);
}

bool UAnthillMenuLibrary::ContinueLastSave(UObject* WorldContextObject)
{
	for (int32 i = 0; i < 3; ++i)
	{
		if (DoesSaveExistInSlot(i))
		{
			return LoadGameFromSlotAndOpenMap(WorldContextObject, i);
		}
	}
	return false;
}
