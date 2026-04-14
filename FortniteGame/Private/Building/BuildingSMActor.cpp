#include "pch.h"
#include "Building/BuildingSMActor.h"

#include <UObject/UObjectGlobals.h>

#include <Items/FortInventory.h>

void BuildingSMActor::AttemptSpawnResources(ABuildingSMActor* Context, __int64 a2, __int64 a3, bool a4)
{
	cout << format("Context: {}\n", Context->GetName());
	cout << format("a2: {}\n", a2);
	cout << format("a3: {}\n", a3);
	cout << format("a4: {}\n", a4);

	cout << format("sizeof a2: {}\n", sizeof(a2));
	cout << format("sizeof a3: {}\n", sizeof(a3));

	cout << format("a2 as UObject: {}\n", Cast<UObject>((void*)a2)->GetName());
	cout << format("a3 as UObject: {}\n", Cast<UObject>((void*)a3)->GetName());
}

void BuildingSMActor::Patch()
{
	VirtualHook<ABuildingSMActor>(0x14E, AttemptSpawnResources);
}
