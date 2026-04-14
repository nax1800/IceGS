#pragma once

class FortAthenaVehicle
{
public:
	static void ServerUpdatePhysicsParams(AFortAthenaVehicle* Context, FReplicatedAthenaVehiclePhysicsState& InState);

	static void Patch();
};

