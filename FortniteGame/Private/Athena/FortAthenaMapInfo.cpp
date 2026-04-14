#include "pch.h"
#include "Athena/FortAthenaMapInfo.h"

FVector* (*FortAthenaMapInfo::PickSupplyDropLocation)(AFortAthenaMapInfo* Context, FVector* result, FVector* CenterLocation, double MaxRadius, bool bAvoidWater, float TraceStartZ, float TraceEndZ) = decltype(FortAthenaMapInfo::PickSupplyDropLocation)(ImageBase + 0xC07DD0);
