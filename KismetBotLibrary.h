#pragma once

/*
	* HUGE DISCLAIMER

	This is not proper bots so they won't function like them, they just are pawns that stand still. 
	Can be used for trickshotting, testing, etc....
*/

static inline int BotCount = 0;

struct FPBot
{
public:
	APlayerPawn_Athena_C* Pawn = nullptr;
	AFortPlayerStateAthena* PlayerState = nullptr;
	AFortPlayerControllerAthena* PlayerController = nullptr;
};

class KismetBotLibrary
{
public:
	static FString GetRandomBotName();
	static FPBot SpawnBot(FVector SpawnLocation);
};

