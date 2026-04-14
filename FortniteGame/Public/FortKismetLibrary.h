#pragma once

struct FFrame;

class FortKismetLibrary
{
public:
    static EFortStructuralGridQueryResults (*CanPlaceBuildableClassInStructuralGrid)(UObject* WorldContextObject, TSubclassOf<ABuildingSMActor> ClassToBuild, const FVector& WorldLocation, const FRotator& WorldRotation,
        bool bMirrored, TArray<ABuildingActor*>& ExistingBuildings, EFortBuildPreviewMarkerOptionalAdjustment* MarkerOptionalAdjustment);

    static void PickLootDrops(UObject* Context, FFrame& Stack, bool* Ret);

    static void Patch();
};

