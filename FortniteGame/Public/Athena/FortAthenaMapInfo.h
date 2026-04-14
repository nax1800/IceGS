#pragma once

class FortAthenaMapInfo
{
public:
	static FVector* (*PickSupplyDropLocation)(AFortAthenaMapInfo* Context, FVector* result, FVector* CenterLocation, double MaxRadius, bool bAvoidWater, float TraceStartZ, float TraceEndZ);
};

