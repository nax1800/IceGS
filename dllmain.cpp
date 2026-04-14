#include "pch.h"

//FortniteGame
#include <FortKismetLibrary.h>

#include <Athena/FortAthenaCreativePortal.h>
#include <Athena/FortAthenaSupplyDrop.h>
#include <Athena/FortAthenaVehicle.h>
#include <Athena/FortGameModeAthena.h>
#include <Athena/FortGameStateAthena.h>
#include <Athena/FortPlayerControllerAthena.h>

#include <Building/BuildingActor.h>
#include <Building/BuildingSMActor.h>
#include <Building/BuildingContainer.h>

#include <Items/FortInventoryOwnerInterface.h>
#include <Items/FortPickup.h>
#include <Items/FortWorldItem.h>

#include <Online/FortGameSession.h>

#include <Pawns/FortPlayerPawn.h>

//Runtime
#include <Engine/NetDriver.h>
#include <Engine/World.h>

#include <UObject/UObjectGlobals.h>

//OnlineGameplayFramework
#include <McpProfileSys/McpProfileGroup.h>

//GameplayAbilities
#include <AbilitySystemComponent.h>
#include <Abilities/GameplayAbility.h>

void Main()
{
#ifdef DEV
    AllocConsole();
    FILE* File;
	freopen_s(&File, "CONOUT$", "w", stdout);
#endif

	if (MH_Initialize() != MH_OK)
        return;

    Sleep(3000);

    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
	UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Athena_Terrain", nullptr);

    *(bool*)(ImageBase + 0x582290C) = false; // GIsClient
    *(bool*)(ImageBase + 0x582290D) = true; // GIsServer

    for (uintptr_t FuncToNull : OffsetsToNull)
    {
		PatchByte(FuncToNull, 0xC3);
    }

#ifdef MATCHMAKING
    EnableMatchmaking();
#endif

    FortKismetLibrary::Patch();

    FortAthenaCreativePortal::Patch();
    FortAthenaSupplyDrop::Patch();
    FortAthenaVehicle::Patch();
    FortPlayerControllerAthena::Patch();
    FortGameModeAthena::Patch();

    BuildingActor::Patch();
    BuildingContainer::Patch();

    FortInventoryOwnerInterface::Patch();
    FortPickup::Patch();
    FortWorldItem::Patch();

    FortGameSession::Patch();

    FortPlayerPawn::Patch();

    NetDriver::Patch();
    World::Patch();

    GarbageCollection::Patch(); // Not a class but oh well makes it cleaner

    McpProfileGroup::Patch();

    AbilitySystemComponent::Patch();
    GameplayAbility::Patch();

	MH_EnableHook(MH_ALL_HOOKS);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		std::thread(Main).detach();

    return TRUE;
}

