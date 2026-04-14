#include "pch.h"
#include "KismetBotLibrary.h"

#include <Items/FortInventory.h>

#include <Engine/World.h>
#include <UObject/UObjectGlobals.h>

FString KismetBotLibrary::GetRandomBotName()
{
	wstring BotName = L"IceBot_" + to_wstring(BotCount);
	return FString(BotName.c_str());
}

FPBot KismetBotLibrary::SpawnBot(FVector SpawnLocation)
{
	FPBot Bot{};

	Bot.Pawn = World::SpawnActor<APlayerPawn_Athena_C>(APlayerPawn_Athena_C::StaticClass(), SpawnLocation);
	if (Bot.Pawn == nullptr)
		return Bot;

	Bot.Pawn->SetPawnVisibility(false, false);

	BotCount++;

	Bot.PlayerController = World::SpawnActor<AAthena_PlayerController_C>(AAthena_PlayerController_C::StaticClass(), SpawnLocation);
	Bot.PlayerState = World::SpawnActor<AFortPlayerStateAthena>(AFortPlayerStateAthena::StaticClass(), SpawnLocation);
	Bot.PlayerController->Player = Cast<UPlayer>(UGameplayStatics::SpawnObject(UPlayer::StaticClass(), Bot.PlayerController));
	Bot.PlayerController->PlayerState = Bot.PlayerState;
	Bot.PlayerController->OnRep_PlayerState();

	Bot.Pawn->PlayerState = Bot.PlayerState;
	Bot.Pawn->OnRep_PlayerState();
	Bot.Pawn->Controller = Bot.PlayerController;

	static USkeletalMesh* JillMesh = StaticFindObject<USkeletalMesh>(nullptr, L"/Game/Characters/Survivors/Female/Small/F_SML_Starter_01/Meshes/F_SML_Starter_Epic.F_SML_Starter_Epic");
	Bot.Pawn->Mesh->SetSkeletalMesh(JillMesh, true);

	Bot.PlayerState->OnRep_CharacterParts();

	Bot.PlayerState->bIsSpectator = false;
	Bot.PlayerState->bIsABot = true;

	Bot.PlayerController->Possess(Bot.Pawn);

	Bot.Pawn->PawnUniqueID = rand() % 1000;
	Bot.Pawn->OnRep_PawnUniqueID();

	Bot.PlayerController->ServerChangeName(GetRandomBotName());

	Bot.PlayerState->bHasFinishedLoading = true;
	Bot.PlayerState->bHasStartedPlaying = true;
	Bot.PlayerState->OnRep_bHasStartedPlaying();
	Bot.PlayerController->ServerSetClientHasFinishedLoading(true);
	Bot.PlayerController->bHasInitiallySpawned = true;
	Bot.PlayerController->bAssignedStartSpawn = true;
	Bot.PlayerController->bReadyToStartMatch = true;
	Bot.PlayerController->bClientPawnIsLoaded = true;
	Bot.PlayerController->bHasClientFinishedLoading = true;
	Bot.PlayerController->bHasServerFinishedLoading = true;
	Bot.PlayerController->OnRep_bHasServerFinishedLoading();

	Bot.Pawn->SetMaxHealth(100.f);
	Bot.Pawn->SetHealth(100.f);

	Bot.Pawn->SetMaxShield(100.f);
	Bot.Pawn->SetShield(100.f);

	static UFortWeaponItemDefinition* ItemDefinition = StaticFindObject<UFortWeaponItemDefinition>(nullptr, L"/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

	UFortWorldItem* WorldItem = Cast<UFortWorldItem>(ItemDefinition->CreateTemporaryItemInstanceBP(1, 1));
	WorldItem->SetOwningControllerForTemporaryItem(Bot.PlayerController);

	Bot.Pawn->EquipWeaponDefinition(ItemDefinition, WorldItem->GetItemGuid());

	EFortTeam Old = Bot.PlayerState->TeamIndex;
	Bot.PlayerState->TeamIndex = EFortTeam::HumanPvP_Team100;
	Bot.PlayerState->OnRep_TeamIndex(Old);

	GetGameState()->PlayersLeft++;
	GetGameState()->OnRep_PlayersLeft();

	GetGameState()->PlayerArray.Add(Bot.PlayerState);
	GetGameMode()->AlivePlayers.Add(Bot.PlayerController);

	return Bot;
}

