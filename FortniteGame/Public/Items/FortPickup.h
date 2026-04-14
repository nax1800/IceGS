#pragma once

class FortPickup
{
public:
	static inline char (*oCompletePickupAnimation)(AFortPickup* Pickup);
	static char CompletePickupAnimation(AFortPickup* Pickup);

	static AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDefinition, int OverrideCount, int LoadedAmmo, FVector Loc, bool bTossedFromContainer = false, AFortPawn* PawnWhoDroppedPickup = nullptr);

	static void Patch();
};