#pragma once

struct FFrame;

class FortAthenaSupplyDrop
{
public:
	static void SpawnPickup(UObject* Context, FFrame& Stack, AFortPickup** Ret);
	static void SpawningLootOnDestruction(UObject* Context, FFrame& Stack);

	static void Patch();
};

