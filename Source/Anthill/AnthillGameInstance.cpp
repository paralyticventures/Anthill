#include "AnthillGameInstance.h"
#include "Anthill/UI/AnthillSaveGame.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UAnthillGameInstance::UAnthillGameInstance()
{
	MainMenuLevel = FSoftObjectPath(TEXT("/Game/Levels/Lvl_MainMenu.Lvl_MainMenu"));
	LoadMenuLevel = FSoftObjectPath(TEXT("/Game/Levels/Lvl_LoadMenu.Lvl_LoadMenu"));
	GameLevel = FSoftObjectPath(TEXT("/Game/FirstPerson/Lvl_FirstPerson.Lvl_FirstPerson"));
}

void UAnthillGameInstance::ResetSessionPlayTime()
{
	RunningPlayTimeSeconds = 0.f;
	ClearPendingLoad();
	ClearCollectedPickupKeys();
}

void UAnthillGameInstance::SetSessionPlayTimeFromSave(float TotalSecondsFromSave)
{
	RunningPlayTimeSeconds = FMath::Max(0.f, TotalSecondsFromSave);
}

void UAnthillGameInstance::AddPlayTimeSeconds(float DeltaSeconds, const UWorld* World)
{
	if (DeltaSeconds <= 0.f || !World || UGameplayStatics::IsGamePaused(World))
	{
		return;
	}
	if (!IsPlaytimeGameplayMap(World))
	{
		return;
	}
	RunningPlayTimeSeconds += DeltaSeconds;
}

bool UAnthillGameInstance::IsPlaytimeGameplayMap(const UWorld* World) const
{
	if (!World)
	{
		return false;
	}
	const FString Current = UGameplayStatics::GetCurrentLevelName(World);
	if (Current.IsEmpty())
	{
		return false;
	}
	// Menu — bez naliczania; każda inna mapa (w tym z zapisu) liczy się jako gra.
	const FString MainName = MainMenuLevel.IsValid() ? MainMenuLevel.GetAssetName() : FString();
	const FString LoadName = LoadMenuLevel.IsValid() ? LoadMenuLevel.GetAssetName() : FString();
	if (!MainName.IsEmpty() && Current == MainName)
	{
		return false;
	}
	if (!LoadName.IsEmpty() && Current == LoadName)
	{
		return false;
	}
	return true;
}

void UAnthillGameInstance::SetPendingLoadFromSaveGame(const UAnthillSaveGame* Save)
{
	ClearPendingLoad();
	if (!Save)
	{
		return;
	}
	PendingLoad.bHasPending = true;
	PendingLoad.bHasTransform = Save->bHasPlayerTransform;
	PendingLoad.Transform = Save->PlayerTransform;
	PendingLoad.bHasCharacterState = Save->bHasCharacterState;
	if (Save->bHasCharacterState)
	{
		PendingLoad.Health = Save->SavedHealth;
		PendingLoad.MaxHealth = Save->SavedMaxHealth;
		PendingLoad.Stamina = Save->SavedStamina;
		PendingLoad.MaxStamina = Save->SavedMaxStamina;
		PendingLoad.InventorySlots = Save->SavedInventorySlots;
		PendingLoad.SelectedInventorySlotIndex = Save->SavedSelectedInventorySlotIndex;
	}
}

void UAnthillGameInstance::ClearPendingLoad()
{
	PendingLoad = FAnthillPendingLoadData();
}

bool UAnthillGameInstance::ConsumePendingLoad(FAnthillPendingLoadData& OutData)
{
	if (!PendingLoad.bHasPending)
	{
		return false;
	}
	OutData = PendingLoad;
	PendingLoad = FAnthillPendingLoadData();
	return true;
}

void UAnthillGameInstance::AddCollectedPickupKey(const FString& Key)
{
	if (!Key.IsEmpty())
	{
		CollectedPickupKeys.Add(Key);
	}
}

bool UAnthillGameInstance::IsPickupKeyCollected(const FString& Key) const
{
	return !Key.IsEmpty() && CollectedPickupKeys.Contains(Key);
}

void UAnthillGameInstance::SetCollectedPickupKeysFromSave(const TArray<FString>& Keys)
{
	CollectedPickupKeys.Reset();
	for (const FString& K : Keys)
	{
		if (!K.IsEmpty())
		{
			CollectedPickupKeys.Add(K);
		}
	}
}

void UAnthillGameInstance::ClearCollectedPickupKeys()
{
	CollectedPickupKeys.Reset();
}

TArray<FString> UAnthillGameInstance::GetCollectedPickupKeysSnapshot() const
{
	TArray<FString> Out;
	Out.Reserve(CollectedPickupKeys.Num());
	for (const FString& K : CollectedPickupKeys)
	{
		Out.Add(K);
	}
	return Out;
}
