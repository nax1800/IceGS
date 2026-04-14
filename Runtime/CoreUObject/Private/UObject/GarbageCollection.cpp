#include "pch.h"
#include "UObject/UObjectGlobals.h"

void CollectGarbage(EObjectFlags KeepFlags, bool bPerformFullPurge)
{
	return;
}

void GarbageCollection::Patch()
{
	CreateHook(0x12F800D, CollectGarbage);
}