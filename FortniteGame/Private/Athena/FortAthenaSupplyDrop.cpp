#include "pch.h"
#include "Athena/FortAthenaSupplyDrop.h"

#include <UObject/UObjectGlobals.h>
#include <UObject/Stack.h>

void FortAthenaSupplyDrop::SpawnPickup(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
    UFortWorldItemDefinition* ItemDefinition = nullptr;
    int NumberToSpawn;
    AFortPawn* TriggeringPawn = nullptr;

    Stack.StepCompiledIn(&ItemDefinition);
    Stack.StepCompiledIn(&NumberToSpawn);
    Stack.StepCompiledIn(&TriggeringPawn);
    Stack.IncrementCode();
    AFortAthenaSupplyDrop* SupplyDrop = Cast<AFortAthenaSupplyDrop>(Context);

    cout << "W\n";

    *Ret =  nullptr; // for now
}

void FortAthenaSupplyDrop::SpawningLootOnDestruction(UObject* Context, FFrame& Stack)
{
    cout << "W2\n";
}

void FortAthenaSupplyDrop::Patch()
{
    ExecHook(AFortAthenaSupplyDrop::StaticClass()->GetFunction("FortAthenaSupplyDrop", "SpawnPickup"), SpawnPickup);
    ExecHook(AFortAthenaSupplyDrop::StaticClass()->GetFunction("FortAthenaSupplyDrop", "SpawningLootOnDestruction"), SpawningLootOnDestruction);
}