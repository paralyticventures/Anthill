#include "AnthillGameInstance.h"

UAnthillGameInstance::UAnthillGameInstance()
{
	MainMenuLevel = FSoftObjectPath(TEXT("/Game/Levels/Lvl_MainMenu.Lvl_MainMenu"));
	LoadMenuLevel = FSoftObjectPath(TEXT("/Game/Levels/Lvl_LoadMenu.Lvl_LoadMenu"));
	GameLevel = FSoftObjectPath(TEXT("/Game/FirstPerson/Lvl_FirstPerson.Lvl_FirstPerson"));
}
