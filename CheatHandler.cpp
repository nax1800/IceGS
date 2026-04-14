#include "pch.h"
#include "CheatHandler.h"

#include <UObject/UObjectGlobals.h>

#include <Items/FortInventory.h>
#include <KismetBotLibrary.h>
#include <Items/FortPickup.h>

void CheatHandler::HandleAlivePlayers(AFortPlayerControllerAthena* Context)
{
	TArray<AFortPlayerControllerAthena*> AlivePlayers = GetGameMode()->AlivePlayers;

	if (AlivePlayers.IsValid() == false)
		return;

	for (AFortPlayerControllerAthena* AlivePlayer : AlivePlayers)
	{
		if (AlivePlayer == nullptr)
			continue;

		AFortPlayerStateAthena* AlivePlayerState = Cast<AFortPlayerStateAthena>(AlivePlayer->PlayerState);
		if (AlivePlayerState == nullptr)
			continue;

		SendMessageToConsole(Context, AlivePlayerState->GetPlayerName());
	}
}

void CheatHandler::HandleGiveAllMats(AFortPlayerControllerAthena* Context)
{
	static UFortResourceItemDefinition* WoodDefinition = StaticFindObject<UFortResourceItemDefinition>(nullptr, L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
	static UFortResourceItemDefinition* StoneDefinition = StaticFindObject<UFortResourceItemDefinition>(nullptr, L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
	static UFortResourceItemDefinition* MetalDefinition = StaticFindObject<UFortResourceItemDefinition>(nullptr, L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

	FortInventory::AddItem(Context->WorldInventory, WoodDefinition, 999);
	FortInventory::AddItem(Context->WorldInventory, StoneDefinition, 999);
	FortInventory::AddItem(Context->WorldInventory, MetalDefinition, 999);
}

void CheatHandler::HandleSpawnItem(AFortPlayerControllerAthena* Context, vector<string> Arguments)
{
	int NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	string& WeaponID = Arguments[1];
	int Count = 1;
	int LoadedAmmo = 0;

	try
	{
		if (NumArgs >= 2)
			Count = std::stoi(Arguments[2]);

		if (NumArgs >= 3)
			LoadedAmmo = std::stoi(Arguments[3]);
	}
	catch (...)
	{
	}

	UFortItemDefinition* WID = StaticFindObject<UFortItemDefinition>(nullptr, wstring(WeaponID.begin(), WeaponID.end()).c_str());
	if (WID == nullptr)
	{
		SendMessageToConsole(Context, L"Weapon ID is invalid.");
		return;
	}

	if (Context->Pawn == nullptr)
		return;

	FortPickup::SpawnPickup(WID, Count, LoadedAmmo, Context->Pawn->K2_GetActorLocation());
}

void CheatHandler::HandleGiveItem(AFortPlayerControllerAthena* Context, vector<string> Arguments)
{
	int NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	string& WeaponID = Arguments[1];
	int Count = 1;
	int LoadedAmmo = 0;

	try
	{
		if (NumArgs >= 2)
			Count = std::stoi(Arguments[2]);

		if (NumArgs >= 3)
			LoadedAmmo = std::stoi(Arguments[3]);
	}
	catch (...)
	{
	}


	UFortItemDefinition* WID = StaticFindObject<UFortItemDefinition>(nullptr, wstring(WeaponID.begin(), WeaponID.end()).c_str());
	if (WID == nullptr)
	{
		SendMessageToConsole(Context, L"Weapon ID is invalid.");
		return;
	}

	if (Context->Pawn == nullptr)
		return;

	if (FortInventory::IsFull(Context->WorldInventory) == false)
		FortInventory::AddItem(Context->WorldInventory, WID, Count, LoadedAmmo);
	else
		FortPickup::SpawnPickup(WID, Count, LoadedAmmo, Context->Pawn->K2_GetActorLocation());
}

void CheatHandler::HandleBuildFree(AFortPlayerControllerAthena* Context)
{
	Context->bBuildFree = ~Context->bBuildFree;
	SendMessageToConsole(Context, std::wstring(L"Build Free set to " + (bool)Context->bBuildFree ? L"true" : L"false").c_str());
}

void CheatHandler::HandleInfiniteAmmo(AFortPlayerControllerAthena* Context)
{
	Context->bInfiniteAmmo = ~Context->bInfiniteAmmo;
	SendMessageToConsole(Context, std::wstring(L"Infinite Ammo set to " + (bool)Context->bInfiniteAmmo ? L"true" : L"false").c_str());
}

void CheatHandler::HandleCheat(AFortPlayerControllerAthena* Context, vector<string> Arguments)
{
	int NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	if (Arguments.size() >= 1)
	{
		string& Command = Arguments[0];
		std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);
		if (Command == "aliveplayers")
		{
			HandleAlivePlayers(Context);
		}
		else if (Command == "giveallmats")
		{
			HandleGiveAllMats(Context);
		}
		else if (Command == "spawnitem")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(Context, L"Please provide a Weapon ID, e.g: /Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");
				return;
			}

			HandleSpawnItem(Context, Arguments);
		}
		else if (Command == "giveitem")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(Context, L"Please provide a Weapon ID, e.g: /Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_C_Ore_T03.WID_Shotgun_Standard_Athena_C_Ore_T03");
				return;
			}

			HandleGiveItem(Context, Arguments);
		}
		else if (Command == "buildfree")
		{
			HandleBuildFree(Context);
		}
		else if (Command == "infiniteammo")
		{
			HandleInfiniteAmmo(Context);
		}

#if defined(CREATIVE) || defined(PLAYGROUND)
		else if (Command == "bot")
		{
			KismetBotLibrary::SpawnBot(Context->Pawn->K2_GetActorLocation());
		}
#endif
		else if (Command == "help")
		{
			SendMessageToConsole(Context, HelpMessage.c_str());
		}
		else
			SendMessageToConsole(Context, L"Could not find Command.");
	}
}