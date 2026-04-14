#include "pch.h"
#include "Building/BuildingFoundation.h"

#include <UObject/Stack.h>

void BuildingFoundation::ShowFoundation(ABuildingFoundation* Context)
{
    if (Context == nullptr)
        return;

	Context->bServerStreamedInLevel = true;

	bool bOldEnabled = Context->bFoundationEnabled;
	Context->bFoundationEnabled = true;
	Context->OnRep_FoundationEnabled(bOldEnabled);

	static auto StartDisabled = 3;
	static auto StartEnabled_Dynamic = 2;
	static auto Static = 0;

	Context->DynamicFoundationType = EDynamicFoundationType::Static;
	Context->OnRep_ServerStreamedInLevel();

	static auto Enabled = 1;
	static auto Disabled = 2;

	Context->OnRep_LevelToStream();

	Context->FlushNetDormancy();
	Context->ForceNetUpdate();
}

void BuildingFoundation::SetFoundationTransform(ABuildingFoundation* Context, const FTransform& Transform)
{
	Context->DynamicFoundationTransform = Transform;
}

void BuildingFoundation::SetDynamicFoundationEnabled(UObject* Context, FFrame& Stack, void* Ret)
{
	bool bEnabled;
	Stack.StepCompiledIn(&bEnabled);
	Stack.IncrementCode();

	ABuildingFoundation* BuildingFoundation = Cast<ABuildingFoundation>(Context);

	ShowFoundation(BuildingFoundation);

	return oSetDynamicFoundationEnabled(Context, Stack, Ret);
}

void BuildingFoundation::SetDynamicFoundationTransform(UObject* Context, FFrame& Stack, void* Ret)
{
	FTransform NewTransform;
	Stack.StepCompiledIn(&NewTransform);

	ABuildingFoundation* BuildingFoundation = Cast<ABuildingFoundation>(Context);

	SetFoundationTransform(BuildingFoundation, NewTransform);

	return oSetDynamicFoundationTransform(Context, Stack, Ret);
}

void BuildingFoundation::Patch()
{
	ExecHook(ABuildingFoundation::StaticClass()->GetFunction("BuildingFoundation", "SetDynamicFoundationEnabled"), SetDynamicFoundationEnabled, oSetDynamicFoundationEnabled);
	ExecHook(ABuildingFoundation::StaticClass()->GetFunction("BuildingFoundation", "SpawningLootOnDestruction"), SetDynamicFoundationTransform, oSetDynamicFoundationTransform);
}
