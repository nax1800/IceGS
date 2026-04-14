#pragma once
#include <string>


class CheatHandler
{
public:
	static inline wstring HelpMessage = LR"(
---------------------------------- MAIN ----------------------------------

cheat aliveplayers - Lists all Alive Players
cheat giveallmats - Gives all materials to player
cheat spawnitem {WID} {Count} {LoadedAmmo} - Spawns a pickup at player
cheat giveitem {WID} {Count} {LoadedAmmo} - Gives the weapon to players inventory
cheat buildfree - Toggle Build Free
cheat infiniteammo - Toggle Infinite Ammo
cheat bot - Spawns a so called bot at the player's location (CREATIVE/PLAYGROUND ONLY)
)";

	static void HandleAlivePlayers(AFortPlayerControllerAthena* Context);
	static void HandleGiveAllMats(AFortPlayerControllerAthena* Context);
	static void HandleSpawnItem(AFortPlayerControllerAthena* Context, vector<string> Arguments);
	static void HandleGiveItem(AFortPlayerControllerAthena* Context, vector<string> Arguments);
	static void HandleBuildFree(AFortPlayerControllerAthena* Context);
	static void HandleInfiniteAmmo(AFortPlayerControllerAthena* Context);

	static void HandleCheat(AFortPlayerControllerAthena* Context, vector<string> Arguments);

};

