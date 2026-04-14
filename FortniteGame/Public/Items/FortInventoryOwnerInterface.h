#pragma once

class FortInventoryOwnerInterface
{
public:
	static bool RemoveInventoryItem(IFortInventoryOwnerInterface* Context, FGuid ItemToRemove, int AmountToRemove, bool a4, bool a5);
	static AFortPlayerControllerAthena* GetPlayerController(IFortInventoryOwnerInterface* Context);
	static void Patch();
};

