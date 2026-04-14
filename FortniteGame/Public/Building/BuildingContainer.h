#pragma once
class BuildingContainer
{
public:
	static bool SpawnLoot(ABuildingContainer* Context, AFortPlayerPawn* PlayerPawn, const EFortPickupSourceTypeFlag InSourceTypeFlag);

	static void Patch();
};

