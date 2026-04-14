#include "pch.h"
#include "Items/FortInventoryOwnerInterface.h"

#include <UObject/UObjectBaseUtility.h>

#include <Items/FortInventory.h>

bool FortInventoryOwnerInterface::RemoveInventoryItem(IFortInventoryOwnerInterface* Context, FGuid ItemToRemove, int AmountToRemove, bool a4, bool a5)
{
	return FortInventory::RemoveItem(GetPlayerController(Context)->WorldInventory, ItemToRemove, AmountToRemove);
}

AFortPlayerControllerAthena* FortInventoryOwnerInterface::GetPlayerController(IFortInventoryOwnerInterface* Context)
{
	return reinterpret_cast<AFortPlayerControllerAthena* (*)(IFortInventoryOwnerInterface*)>(Context->VTable[1])(Context);
}

void FortInventoryOwnerInterface::Patch()
{
	UObject* InterfaceAddress = Cast<UObject>(UObjectBaseUtility::GetInterfaceAddress(AFortPlayerControllerAthena::GetDefaultObj(), IFortInventoryOwnerInterface::StaticClass()));
	VirtualHook(InterfaceAddress, 0x14, RemoveInventoryItem);
}