#pragma once

class FortInventory
{
public:
	static EFortQuickBars GetQuickBars(UFortItemDefinition* ItemDefinition);
	static bool IsFull(AFortInventory* Context);
	static FFortItemEntry* FindItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition);
	static FFortItemEntry* FindItem(AFortInventory* Context, FGuid ItemGuid);
	static UFortWorldItem* CreateItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count = 1);
	static bool RemoveItem(AFortInventory* Context, FGuid ItemGuid, int Count = -1);
	static bool RemoveItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count = -1);
	static FGuid AddItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count = 1, int LoadedAmmo = 0, bool bForceNewItem = false);
};