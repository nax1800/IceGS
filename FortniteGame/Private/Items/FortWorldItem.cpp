#include "pch.h"
#include "Items/FortWorldItem.h"

bool FortWorldItem::SetLoadedAmmo(UFortWorldItem* Context, int InCount)
{
    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Context->GetOwningController());
	for (FFortItemEntry& ReplicatedEntry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
    {
        if (UKismetGuidLibrary::EqualEqual_GuidGuid(ReplicatedEntry.ItemGuid, Context->GetItemGuid()))
        {
            ReplicatedEntry.LoadedAmmo = InCount;
            Context->ItemEntry.LoadedAmmo = InCount;
            PlayerController->WorldInventory->Inventory.MarkArrayDirty();
            PlayerController->WorldInventory->Inventory.MarkItemDirty(ReplicatedEntry);
            Context->ItemEntry.bIsDirty = true;
            return true;
        }
    }

    return false;
}

void FortWorldItem::Patch()
{
    VirtualHook<UFortWorldItem>(0x4A7, SetLoadedAmmo);
   // CreateHook(0x2A7140, SetLoadedAmmo);
}