#include "pch.h"
#include "Athena/FortAthenaCreativePortal.h"

#include <UObject/UObjectGlobals.h>
#include <UObject/Stack.h>

#include <Items/FortInventory.h>

void FortAthenaCreativePortal::TeleportPlayerToLinkedVolume(UObject* Context, FFrame& Stack)
{
    AFortPlayerPawnAthena* PlayerPawn = nullptr;
    bool bUseSpawnTags;

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.StepCompiledIn(&bUseSpawnTags);
    Stack.IncrementCode();
    AFortAthenaCreativePortal* Portal = Cast<AFortAthenaCreativePortal>(Context);

    if (PlayerPawn == nullptr)
        return;

    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);
    static UFortWeaponItemDefinition* CreativePhone = StaticFindObject<UFortWeaponItemDefinition>(nullptr, L"/Game/Athena/Items/Weapons/Prototype/WID_CreativeTool.WID_CreativeTool");

    if (FortInventory::FindItem(PlayerController->WorldInventory, CreativePhone) == nullptr)
        FortInventory::AddItem(PlayerController->WorldInventory, CreativePhone, 1, 1);   

    FVector Location = Portal->LinkedVolume->K2_GetActorLocation();
    Location.Z = 10000;

    PlayerController->CreativePlotLinkedVolume = Portal->LinkedVolume;
    PlayerController->OnRep_CreativePlotLinkedVolume();

    PlayerPawn->K2_TeleportTo(Location, FRotator());
    PlayerPawn->BeginSkydiving(false);
}

void FortAthenaCreativePortal::TeleportPlayer(UObject* Context, FFrame& Stack)
{
	AFortAthenaCreativePortal* Portal = Cast<AFortAthenaCreativePortal>(Context);

	AFortPlayerPawn* PlayerPawn = nullptr;
	FRotator TeleportRotation;

	Stack.StepCompiledIn(&PlayerPawn);
	Stack.StepCompiledIn(&TeleportRotation);

	if (PlayerPawn == nullptr)
		return;

	if (Portal->bReturnToCreativeHub)
	{
		AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(PlayerPawn->Controller);

		if (Controller == nullptr)
			return;

		Controller->ServerTeleportToPlaygroundLobbyIsland();
	}
	else
		PlayerPawn->K2_TeleportTo(Portal->TeleportLocation, TeleportRotation);
}

void FortAthenaCreativePortal::Patch()
{
    ExecHook(AFortAthenaCreativePortal::StaticClass()->GetFunction("FortAthenaCreativePortal", "TeleportPlayerToLinkedVolume"), TeleportPlayerToLinkedVolume);
    ExecHook(AFortAthenaCreativePortal::StaticClass()->GetFunction("FortAthenaCreativePortal", "TeleportPlayer"), TeleportPlayer);
}
