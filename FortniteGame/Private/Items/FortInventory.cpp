#include "pch.h"
#include "Items/FortInventory.h"

#include <Items/FortPickup.h>

EFortQuickBars FortInventory::GetQuickBars(UFortItemDefinition* ItemDefinition)
{
	if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
		return EFortQuickBars::Primary;

	return EFortQuickBars::Secondary;
}

bool FortInventory::IsFull(AFortInventory* Context)
{
	int Slots = 0;
	for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (Context->Inventory.ReplicatedEntries[i].ItemDefinition == nullptr)
			continue;

		if (GetQuickBars(Context->Inventory.ReplicatedEntries[i].ItemDefinition) == EFortQuickBars::Primary)
		{
			Slots++;

			if (Slots >= 5)
			{
				break;
			}
		}
	}

	return Slots >= 5;
}

FFortItemEntry* FortInventory::FindItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition)
{
	if (Context->Inventory.ReplicatedEntries.IsValid() == false)
		return nullptr;

	for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (Context->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDefinition)
			return &Context->Inventory.ReplicatedEntries[i];
	}

	return nullptr;
}

FFortItemEntry* FortInventory::FindItem(AFortInventory* Context, FGuid ItemGuid)
{
	if (Context->Inventory.ReplicatedEntries.IsValid() == false)
		return nullptr;

	for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (UKismetGuidLibrary::EqualEqual_GuidGuid(Context->Inventory.ReplicatedEntries[i].ItemGuid, ItemGuid))
			return &Context->Inventory.ReplicatedEntries[i];
	}

	return nullptr;
}

UFortWorldItem* FortInventory::CreateItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count)
{
	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1));
	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(Context->Owner);
	WorldItem->SetOwningControllerForTemporaryItem(PlayerController);
	return WorldItem;
}

bool FortInventory::RemoveItem(AFortInventory* Context, FGuid ItemGuid, int Count)
{
	bool bWasSuccessful = false;
	for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (UKismetGuidLibrary::EqualEqual_GuidGuid(Context->Inventory.ReplicatedEntries[i].ItemGuid, ItemGuid))
		{
			if (Count == -1 || Count >= Context->Inventory.ReplicatedEntries[i].Count)
			{
				Context->Inventory.ReplicatedEntries.Remove(i);
				break;
			}

			Context->Inventory.ReplicatedEntries[i].Count -= Count;
			Context->Inventory.MarkItemDirty(Context->Inventory.ReplicatedEntries[i]);
			bWasSuccessful = true;
		}
	}

	Context->Inventory.MarkArrayDirty();
	Context->HandleInventoryLocalUpdate();

	return bWasSuccessful;
}

bool FortInventory::RemoveItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count)
{
	for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (Context->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDefinition)
			return RemoveItem(Context, Context->Inventory.ReplicatedEntries[i].ItemGuid, Count);
	}

	return false;
}

FGuid FortInventory::AddItem(AFortInventory* Context, UFortItemDefinition* ItemDefinition, int Count, int LoadedAmmo, bool bForceNewItem)
{
	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(Context->Owner);
	bool bAllowMultipleStacks = ItemDefinition->bAllowMultipleStacks;
	int32 MaxStackSize = ItemDefinition->MaxStackSize;
	UFortWorldItem* StackingItemInstance = nullptr;
	int OverStack = 0;

	if (MaxStackSize > 1)
	{
		for (int i = 0; i < Context->Inventory.ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry CurrentEntry = Context->Inventory.ReplicatedEntries[i];
			UFortItemDefinition* CurrentReplicatedEntry = CurrentEntry.ItemDefinition;

			if (CurrentEntry.ItemDefinition == ItemDefinition)
			{
				if (CurrentEntry.Count < MaxStackSize || !bAllowMultipleStacks)
				{
					StackingItemInstance = Cast<UFortWorldItem>(CurrentReplicatedEntry);

					OverStack = CurrentEntry.Count + Count - MaxStackSize;

					if (bAllowMultipleStacks == false && CurrentEntry.Count > MaxStackSize)
						break;

					int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

					FFortItemEntry* ReplicatedEntry = FindItem(Context, CurrentEntry.ItemGuid);

					CurrentEntry.Count += AmountToStack;
					ReplicatedEntry->Count += AmountToStack;

					if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) == true || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) == true)
					{
						FFortItemEntryStateValue Value{};
						Value.IntValue = 1;
						Value.StateType = EFortItemEntryState::ShouldShowItemToast;
						ReplicatedEntry->StateValues.Add(Value);
					}

					Context->Inventory.MarkItemDirty(CurrentEntry);
					Context->Inventory.MarkItemDirty(*ReplicatedEntry);

					if (OverStack <= 0)
						return ReplicatedEntry->ItemGuid;
				}
			}
		}
	}

	Count = OverStack > 0 ? OverStack : Count;

	if (OverStack > 0 && ItemDefinition->bAllowMultipleStacks == false || OverStack > 0 && IsFull(Context) == true)
	{
		APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(PlayerController->AcknowledgedPawn);

		if (Pawn == nullptr)
			return FGuid(-1, -1, -1, -1);

		FortPickup::SpawnPickup(ItemDefinition, Count, LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
		return FGuid(-1, -1, -1, -1);
	}

	UFortWorldItem* NewWorldItem = CreateItem(Context, ItemDefinition, Count);
	if (NewWorldItem == nullptr)
		return FGuid(-1, -1, -1, -1);

	NewWorldItem->ItemEntry.LoadedAmmo = LoadedAmmo;
	if (ItemDefinition->MaxStackSize < Count)
	{
		PlayerController->ServerAttemptInventoryDrop(NewWorldItem->ItemEntry.ItemGuid, Count - ItemDefinition->MaxStackSize);
		NewWorldItem->ItemEntry.Count = ItemDefinition->MaxStackSize;
	}

	if (ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) == true || ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) == true)
	{
		FFortItemEntryStateValue Value{};
		Value.IntValue = 1;
		Value.StateType = EFortItemEntryState::ShouldShowItemToast;
		NewWorldItem->ItemEntry.StateValues.Add(Value);
	}

	Context->Inventory.ReplicatedEntries.Add(NewWorldItem->ItemEntry);
	Context->HandleInventoryLocalUpdate();
	Context->Inventory.MarkArrayDirty();
	Context->Inventory.MarkItemDirty(NewWorldItem->ItemEntry);

	return NewWorldItem->ItemEntry.ItemGuid;
}