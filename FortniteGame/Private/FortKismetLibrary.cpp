#include "pch.h"
#include "FortKismetLibrary.h"

#include <UObject/Stack.h>

#include <Athena/FortGameModeAthena.h>

#include <KismetLootLibrary.h>

EFortStructuralGridQueryResults (*FortKismetLibrary::CanPlaceBuildableClassInStructuralGrid)(UObject* WorldContextObject, TSubclassOf<ABuildingSMActor> ClassToBuild, const FVector& WorldLocation, const FRotator& WorldRotation,
    bool bMirrored, TArray<ABuildingActor*>& ExistingBuildings, EFortBuildPreviewMarkerOptionalAdjustment* MarkerOptionalAdjustment) = decltype(FortKismetLibrary::CanPlaceBuildableClassInStructuralGrid)(ImageBase + 0xF61000);

void FortKismetLibrary::PickLootDrops(UObject* Context, FFrame& Stack, bool* Ret)
{
    UObject* WorldContextObject = nullptr;
    TArray<FFortItemEntry>* OutLootToDrop;
    FName TierGroupName;
    int32 WorldLevel;
    int32 ForcedLootTier;

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&OutLootToDrop);
	Stack.StepCompiledIn(&TierGroupName);
	Stack.StepCompiledIn(&WorldLevel);
	Stack.StepCompiledIn(&ForcedLootTier);
    Stack.IncrementCode();

	cout << "Sigma\n";

	vector<LootDrop> LootDrops = UKismetLootLibrary::PickLootDrops(TierGroupName, GetGameState()->WorldLevel, ForcedLootTier);
	TArray<FFortItemEntry> ok;


	for (LootDrop LootDrop : LootDrops)
	{
		if (LootDrop.ItemEntry == nullptr)
			continue;

		ok.Add(*LootDrop.ItemEntry);
	}

	OutLootToDrop = &ok;

	ok.Free();

	*Ret = true;
}

void FortKismetLibrary::Patch()
{
	ExecHook(UFortKismetLibrary::StaticClass()->GetFunction("FortKismetLibrary", "PickLootDrops"), PickLootDrops);
}