#include "pch.h"
#include "Building/BuildingContainer.h"

#include "KismetLootLibrary.h"
#include <Athena/FortGameModeAthena.h>
#include <Items/FortPickup.h>

bool BuildingContainer::SpawnLoot(ABuildingContainer* Context, AFortPlayerPawn* PlayerPawn, const EFortPickupSourceTypeFlag InSourceTypeFlag)
{
	FVector SpawnLoc = Context->K2_GetActorLocation() + Context->GetActorRightVector() * 70.f + FVector{ 0, 0, 50 };
	vector<LootDrop> LootDrops = UKismetLootLibrary::PickLootDrops(FortGameModeAthena::RedirectLootTierGroup(Context->SearchLootTierGroup), GetGameState()->WorldLevel);

	for (LootDrop LootDrop : LootDrops)
	{
		FortPickup::SpawnPickup(LootDrop.ItemEntry->ItemDefinition, LootDrop.ItemEntry->Count, LootDrop.ItemEntry->LoadedAmmo, SpawnLoc, InSourceTypeFlag == EFortPickupSourceTypeFlag::Container);
	}

	Context->bAlreadySearched = true;
	Context->OnRep_bAlreadySearched();

	return true;
}

void BuildingContainer::Patch()
{
	CreateHook(0xD26D00, SpawnLoot);
}
