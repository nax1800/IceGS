#pragma once

class FortGameModeAthena
{
private:
	static inline bool (*oReadyToStartMatch)(AFortGameModeAthena* Context);
	static inline void (*oHandleStartingNewPlayer)(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer);

public:
	static void (*RemoveFromAlivePlayers)(AFortGameModeAthena* GameMode, AFortPlayerController* PlayerController, APlayerState* PlayerState, APawn* FinisherPawn, UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause, char a7);

	static FName RedirectLootTierGroup(FName LootTierGroup);

	static bool ReadyToStartMatch(AFortGameModeAthena* Context);
	static APawn* SpawnDefaultPawnFor(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot);
	static void HandleStartingNewPlayer(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer);
	static uint8_t PickTeam(AFortGameModeAthena* Context, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller);

	static void Patch();
};

