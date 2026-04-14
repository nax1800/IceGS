#include "pch.h"
#include "Items/FortPickup.h"

#include <Items/FortInventory.h>
#include <Engine/World.h>

char FortPickup::CompletePickupAnimation(AFortPickup* Pickup)
{
	if (Pickup == nullptr)
		return oCompletePickupAnimation(Pickup);

	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Pickup->PickupLocationData.PickupTarget);
	if (Pawn == nullptr)
		return oCompletePickupAnimation(Pickup);

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Pawn->GetController());
	if (PlayerController == nullptr)
		return oCompletePickupAnimation(Pickup);

	FGuid Swap = FGuid(-1, -1, -1, -1);

	FFortItemEntry* PickupEntry = &Pickup->PrimaryPickupItemEntry;
	FFortItemEntry* InvItemEntry = FortInventory::FindItem(PlayerController->WorldInventory, Pickup->PickupLocationData.PickupGuid);

	if (PickupEntry == nullptr || InvItemEntry == nullptr)
	{
		SpawnPickup(PickupEntry->ItemDefinition, PickupEntry->Count, PickupEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
		return oCompletePickupAnimation(Pickup);
	}

	UFortWorldItemDefinition* PickupItemDefinition = reinterpret_cast<UFortWorldItemDefinition*>(PickupEntry->ItemDefinition);
	int IncomingCount = PickupEntry->Count;
	FGuid ItemGuid = PickupEntry->ItemGuid;

	if (PickupItemDefinition == nullptr)
		return oCompletePickupAnimation(Pickup);

	if (FortInventory::GetQuickBars(PickupItemDefinition) == EFortQuickBars::Primary)
	{
		int NewCount = 0;
		int OverStack = 0;

		bool bSuccess = false;
		bool bDrop = false;

		for (FFortItemEntry& CurrentEntry : PlayerController->WorldInventory->Inventory.ReplicatedEntries)
		{
			if (CurrentEntry.ItemDefinition == PickupItemDefinition)
			{
				NewCount = CurrentEntry.Count + IncomingCount;
				OverStack = NewCount - PickupItemDefinition->MaxStackSize;

				CurrentEntry.Count = OverStack > 0 ? NewCount - OverStack : NewCount;

				PlayerController->WorldInventory->Inventory.MarkItemDirty(CurrentEntry);
				PlayerController->WorldInventory->Inventory.MarkArrayDirty();

				if (OverStack <= 0)
				{
					bSuccess = true;
					break;
				}
			}
		}

		bDrop = !bSuccess;

		if (FortInventory::IsFull(PlayerController->WorldInventory) == true && bDrop == true)
		{
			int DropCount = OverStack > 0 ? OverStack : IncomingCount;

			if (Cast<UFortWorldItemDefinition>(InvItemEntry->ItemDefinition)->bCanBeDropped == true && InvItemEntry->ItemDefinition->bAllowMultipleStacks == true)
			{
				PlayerController->ServerAttemptInventoryDrop(InvItemEntry->ItemGuid, InvItemEntry->Count);

				Swap = FortInventory::AddItem(PlayerController->WorldInventory, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
			}
			else
			{
				SpawnPickup(PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
			}
		}
		else if (FortInventory::IsFull(PlayerController->WorldInventory) == false && bDrop == true)
		{
			int DropCount = OverStack > 0 ? OverStack : IncomingCount;

			FortInventory::AddItem(PlayerController->WorldInventory, PickupItemDefinition, DropCount, PickupEntry->LoadedAmmo);
		}
	}
	else
		FortInventory::AddItem(PlayerController->WorldInventory, PickupItemDefinition, IncomingCount, PickupEntry->LoadedAmmo);

	FGuid CurrentGuid = FGuid(-1, -1, -1, -1);
	if (Pawn->CurrentWeapon != nullptr)
		CurrentGuid = Pawn->CurrentWeapon->ItemEntryGuid;

	if (UKismetGuidLibrary::NotEqual_GuidGuid(Swap, FGuid(-1, -1, -1, -1)) && UKismetGuidLibrary::EqualEqual_GuidGuid(CurrentGuid, Pickup->PickupLocationData.PickupGuid))
		PlayerController->ClientEquipItem(Swap, false);

	return oCompletePickupAnimation(Pickup);
}

AFortPickupAthena* FortPickup::SpawnPickup(UFortItemDefinition* ItemDefinition, int OverrideCount, int LoadedAmmo, FVector Loc, bool bTossedFromContainer, AFortPawn* PawnWhoDroppedPickup)
{
	if (!ItemDefinition)
		return nullptr;

	FTransform SpawnTransform = FTransform();
	SpawnTransform.Scale3D = FVector(1, 1, 1);
	SpawnTransform.Translation = Loc;

	auto SpawnedPickup = World::SpawnActor<AFortPickupAthena>(AFortPickupAthena::StaticClass(), SpawnTransform);
	if (!SpawnedPickup)
		return nullptr;

	FFortItemEntry* PickupEntry = &SpawnedPickup->PrimaryPickupItemEntry;
	PickupEntry->ItemDefinition = ItemDefinition;
	PickupEntry->Count = OverrideCount;
	PickupEntry->LoadedAmmo = LoadedAmmo;
	PickupEntry->ReplicationKey++;
	SpawnedPickup->OnRep_PrimaryPickupItemEntry();

	SpawnedPickup->TossPickup(Loc, nullptr, 1, true, EFortPickupSourceTypeFlag::Other);

	SpawnedPickup->SetReplicateMovement(true);
	SpawnedPickup->MovementComponent = Cast<UProjectileMovementComponent>(UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), SpawnedPickup));

	SpawnedPickup->PawnWhoDroppedPickup = PawnWhoDroppedPickup;

	SpawnedPickup->bTossedFromContainer = bTossedFromContainer;
	SpawnedPickup->OnRep_TossedFromContainer();

	return SpawnedPickup;
}

void FortPickup::Patch()
{
	CreateHook(0x10745D0, CompletePickupAnimation, (void**)&oCompletePickupAnimation);
}