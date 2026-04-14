#include "pch.h"
#include "Athena/FortGameModeAthena.h"

#include <AbilitySystemComponent.h>

#include <PlaysetLevelStreamComponent.h>
#include <Athena/FortAthenaMapInfo.h>
#include <Building/BuildingContainer.h>
#include <Items/FortInventory.h>

#include <IpNetDriver.h>
#include <Engine/Engine.h>
#include <Engine/NetDriver.h>
#include <Engine/World.h>
#include <UObject/UObjectGlobals.h>
#include <UObject/UObjectBase.h>
#include <Building/BuildingFoundation.h>


void (*FortGameModeAthena::RemoveFromAlivePlayers)(AFortGameModeAthena* GameMode, AFortPlayerController* PlayerController, APlayerState* PlayerState, APawn* FinisherPawn, UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause, char a7) = decltype(FortGameModeAthena::RemoveFromAlivePlayers)(ImageBase + 0xC3AE10);

FName FortGameModeAthena::RedirectLootTierGroup(FName LootTierGroup)
{
	static FName NAME_Loot_Treasure = UKismetStringLibrary::Conv_StringToName(L"Loot_Treasure");
	static FName NAME_Loot_Ammo = UKismetStringLibrary::Conv_StringToName(L"Loot_Ammo");
	static FName NAME_Loot_AthenaTreasure = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure");
	static FName NAME_Loot_AthenaAmmoLarge = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge");

	if (LootTierGroup == NAME_Loot_Treasure)
		return NAME_Loot_AthenaTreasure;

	if (LootTierGroup == NAME_Loot_Ammo)
		return NAME_Loot_AthenaAmmoLarge;

	return LootTierGroup;
}

static void StreamLevel(const std::string& LevelName, FVector Location = {})
{
	FTransform Transform{};
	Transform.Scale3D = { 1, 1, 1 };
	Transform.Translation = Location;
	auto BuildingFoundation = World::SpawnActor<ABuildingFoundation3x3>(ABuildingFoundation3x3::StaticClass(), Transform);

	if (!BuildingFoundation)
		return;

	FBuildingFoundationStreamingData& StreamingData = BuildingFoundation->StreamingData;

	StreamingData.FoundationName = UKismetStringLibrary::Conv_StringToName(std::wstring(LevelName.begin(), LevelName.end()).c_str());
	StreamingData.FoundationLocation = Location;

	BuildingFoundation->LevelToStream = UKismetStringLibrary::Conv_StringToName(std::wstring(LevelName.begin(), LevelName.end()).c_str());

	BuildingFoundation->OnRep_LevelToStream();

	BuildingFoundation::ShowFoundation(BuildingFoundation);
}

bool FortGameModeAthena::ReadyToStartMatch(AFortGameModeAthena* Context)
{
	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(Context->GameState);

	if (GameState->CurrentPlaylistInfo.BasePlaylist == nullptr)
	{
#ifdef CREATIVE
		UFortPlaylistAthena* Playlist = StaticFindObject<UFortPlaylistAthena>(nullptr, L"/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2");
#elifdef PLAYGROUND
		UFortPlaylistAthena* Playlist = StaticFindObject<UFortPlaylistAthena>(nullptr, L"/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground");
#else
		UFortPlaylistAthena* Playlist = StaticFindObject<UFortPlaylistAthena>(nullptr, L"/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
#endif

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.IncrementArrayReplicationKey();
		GameState->CurrentPlaylistInfo.MarkArrayDirty();
		GameState->OnRep_CurrentPlaylistInfo();
		
		for (TSoftObjectPtr<UWorld> AdditionalLevel : Playlist->AdditionalLevels)
		{
			//UGameplayStatics::LoadStreamLevel(UWorld::GetWorld(), AdditionalLevel.ObjectID.AssetPathName, true, false, {});

			GameState->AdditionalPlaylistLevelsStreamed.Add(AdditionalLevel.ObjectID.AssetPathName);
			StreamLevel(AdditionalLevel.ObjectID.AssetPathName.ToString());
		}
		GameState->OnRep_AdditionalPlaylistLevelsStreamed();

		BuildingFoundation::ShowFoundation(StaticFindObject<ABuildingFoundation>(nullptr, L"/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.LF_Athena_POI_25x36"));
		BuildingFoundation::ShowFoundation(StaticFindObject<ABuildingFoundation>(nullptr, L"/Game/Athena/Maps/Athena_POI_Foundations.Athena_POI_Foundations.PersistentLevel.ShopsNew"));

		GameState->CurrentPlaylistId = Playlist->PlaylistId;
		GameState->OnRep_CurrentPlaylistId();

		GameState->AirCraftBehavior = Playlist->AirCraftBehavior;

		Context->CurrentPlaylistId = Playlist->PlaylistId;
		Context->CurrentPlaylistName = Playlist->PlaylistName;

		GameState->WorldLevel = Playlist->LootLevel;
	}

	if (GameState->MapInfo == nullptr)
		return false;

	if (Context->bWorldIsReady != true)
	{
		FName NAME_GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
		UWorld* World = UWorld::GetWorld();

		UNetDriver* NetDriver = Engine::CreateNetDriver(UFortEngine::GetEngine(), World, NAME_GameNetDriver);
		if (NetDriver == nullptr)
			return false;

		NetDriver->World = World;
		NetDriver->NetDriverName = NAME_GameNetDriver;

		World->NetDriver = NetDriver;

		FString Error;
		FURL URL{};
		URL.Port = 7777;

		if (IpNetDriver::InitListen(NetDriver, World, URL, false, Error) == false)
			return false;

		NetDriver::SetWorld(NetDriver, World);

		for (FLevelCollection& LevelCollection : World->LevelCollections)
		{
			LevelCollection.NetDriver = NetDriver;
		}

		GameState->OnRep_CurrentPlaylistInfo();

		Context->GameSession->MaxPlayers = 100;
		Context->WarmupRequiredPlayerCount = 1;

		Context->bWorldIsReady = true;
	}

	return oReadyToStartMatch(Context);
}

APawn* FortGameModeAthena::SpawnDefaultPawnFor(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
	for (FItemAndCount& StartingItem : Context->StartingItems)
	{
		FortInventory::AddItem(NewPlayer->WorldInventory, StartingItem.Item, StartingItem.Count);
	}
	FortInventory::AddItem(NewPlayer->WorldInventory, NewPlayer->CustomizationLoadout.Pickaxe->WeaponDefinition);

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(Context->GameState);
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

	if (GameState->CurrentPlaylistInfo.BasePlaylist != nullptr)
	{
		TArray<TSoftObjectPtr<UFortGameplayModifierItemDefinition>>& ModifierList = GameState->CurrentPlaylistInfo.BasePlaylist->ModifierList;

		for (TSoftObjectPtr<UFortGameplayModifierItemDefinition>& Modifier : ModifierList)
		{
			UFortGameplayModifierItemDefinition* StrongModifier = Modifier.Get();
			if (StrongModifier == nullptr)
				continue;

			UAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

			for (FFortAbilitySetDeliveryInfo& PersistentAbilitySet : StrongModifier->PersistentAbilitySets)
			{
				if (PersistentAbilitySet.DeliveryRequirements.bApplyToPlayerPawns == false)
					continue;

				TArray<TSoftObjectPtr<UFortAbilitySet>>& CurrentAbilitySets = PersistentAbilitySet.AbilitySets;

				for (TSoftObjectPtr<UFortAbilitySet>& CurrentAbilitySet : CurrentAbilitySets)
				{
					if (UObjectBase::IsValidLowLevel(CurrentAbilitySet.Get()) == false)
						continue;

					for (TSubclassOf<UFortGameplayAbility> GameplayAbility : CurrentAbilitySet.Get()->GameplayAbilities)
					{
						AbilitySystemComponent::GrantGameplayAbility(AbilitySystemComponent, GameplayAbility.Get());
					}
				}
			}

			for (FFortGameplayEffectDeliveryInfo& GameplayEffectDeliveryInfo : Modifier->PersistentGameplayEffects)
			{
				if (GameplayEffectDeliveryInfo.DeliveryRequirements.bApplyToPlayerPawns == false)
					continue;

				TArray<FGameplayEffectApplicationInfo>& CurrentGameplayEffects = GameplayEffectDeliveryInfo.GameplayEffects;

				for (FGameplayEffectApplicationInfo& CurrentGameplayEffectInfo : CurrentGameplayEffects)
				{
					if (!CurrentGameplayEffectInfo.GameplayEffect.Get())
						continue;

					AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(CurrentGameplayEffectInfo.GameplayEffect.Get(), CurrentGameplayEffectInfo.Level, {});
				}
			}
		}
	}

	return Context->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void FortGameModeAthena::HandleStartingNewPlayer(AFortGameModeAthena* Context, AFortPlayerControllerAthena* NewPlayer)
{
	oHandleStartingNewPlayer(Context, NewPlayer);

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(Context->GameState);

	if (0)
	{
		float LlamaMin = EvaluateCurveTable(GameState->MapInfo->LlamaQuantityMin);
		float LlamaMax = EvaluateCurveTable(GameState->MapInfo->LlamaQuantityMax);
		int32 LlamaCount = UKismetMathLibrary::RandomIntegerInRange((int)LlamaMin, (int)LlamaMax);
		float Radius = 120000;

		FVector Center{};
		Center.Z = 10000;

		for (int i = 0; i < LlamaCount; i++)
		{
			FVector Loc(0, 0, 0);
			FortAthenaMapInfo::PickSupplyDropLocation(GameState->MapInfo, &Loc, &Center, Radius, 0, -1, -1);

			if (Loc.X != 0 || Loc.Y != 0 || Loc.Z != 0)
			{
				FQuat Rot{};
				Rot.X = (float)rand() * 0.010986663f;

				FTransform Transform{};
				Transform.Translation = Loc;
				Transform.Rotation = Rot;
				Transform.Scale3D = { 1, 1, 1 };

				AFortAthenaSupplyDrop* Llama = Cast<AFortAthenaSupplyDrop>(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), GameState->MapInfo->LlamaClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr));
				FVector GroundLoc = Llama->FindGroundLocationAt(Loc);

				UGameplayStatics::FinishSpawningActor(Llama, FTransform(Rot, Llama->FindGroundLocationAt(Loc), { 1, 1, 1 }));
			}
		}
	}

#ifdef CHRISTMASBUS
	static UAthenaBattleBusItemDefinition* BattleBusDefinition = StaticFindObject<UAthenaBattleBusItemDefinition>(nullptr, L"/Game/Athena/Items/Cosmetics/BattleBuses/BBID_WinterBus.BBID_WinterBus");
	if (GameState->DefaultBattleBus != BattleBusDefinition)
	{
		GameState->DefaultBattleBus = BattleBusDefinition;

		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortAthenaAircraft::StaticClass(), &OutActors);

		for (AActor* OutActor : OutActors)
		{
			if (OutActor == nullptr)
				continue;

			AFortAthenaAircraft* Aircraft = Cast<AFortAthenaAircraft>(OutActor);
			Aircraft->DefaultBusSkin = BattleBusDefinition;

			if (Aircraft->SpawnedCosmeticActor != nullptr)
				Aircraft->SpawnedCosmeticActor->ActiveSkin = BattleBusDefinition;
		}

		OutActors.Free();
	}
#endif

#ifdef CREATIVE
    AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(NewPlayer->PlayerState);

    AFortAthenaCreativePortal* Portal = nullptr;

    if (GameState->CreativePortalManager->AvailablePortals.Num() > 0)
    {
        Portal = Cast<AFortAthenaCreativePortal>(GameState->CreativePortalManager->AvailablePortals[0]);
        GameState->CreativePortalManager->AvailablePortals.Remove(0);
        GameState->CreativePortalManager->UsedPortals.Add(Portal);
    }

    if (Portal == nullptr)
        return;

	Portal->PlayersReady.Add(PlayerState->UniqueId);
	Portal->OnRep_PlayersReady();

    Portal->OwningPlayer = PlayerState->UniqueId;
    Portal->OnRep_OwningPlayer();

	Portal->CachedOwningPlayerState = PlayerState;

    Portal->bPortalOpen = true;
    Portal->OnRep_PortalOpen();

	Portal->bUserInitiatedLoad = true;
	Portal->bInErrorState = false;

	UFortLevelSaveComponent* LevelSaveComponent = Cast<UFortLevelSaveComponent>(Portal->LinkedVolume->GetComponentByClass(UFortLevelSaveComponent::StaticClass()));

    if (LevelSaveComponent != nullptr)
    {
        LevelSaveComponent->AccountIdOfOwner = PlayerState->UniqueId;
        LevelSaveComponent->bIsLoaded = true;
		LevelSaveComponent->bIsActive = true;
		LevelSaveComponent->bLoadPlaysetFromPlot = true;

		LevelSaveComponent->OnRep_IsActive();
		LevelSaveComponent->OnRep_Loaded();
    }

	static UFortPlaysetItemDefinition* IslandPlayset = StaticFindObject<UFortPlaysetItemDefinition>(nullptr, L"/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");

	UPlaysetLevelStreamComponent* LevelStreamComponent = Cast<UPlaysetLevelStreamComponent>(Portal->LinkedVolume->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass()));
    LevelStreamComponent->SetPlayset(IslandPlayset);
	LevelStreamComponent->CurrentPlayset = IslandPlayset;

	PlaysetLevelStreamComponent::LoadPlayset(LevelStreamComponent);

    Portal->LinkedVolume->VolumeState = EVolumeState::Ready;
    Portal->LinkedVolume->OnRep_VolumeState();

    NewPlayer->OwnedPortal = Portal;

	NewPlayer->VolumesLoading.Add(Portal->LinkedVolume);

	NewPlayer->CreativePlotLinkedVolume = Portal->LinkedVolume;
	NewPlayer->OnRep_CreativePlotLinkedVolume();
#else
	static bool bSpawnedFloorLoot = false;
	if (bSpawnedFloorLoot == false)
	{
		bSpawnedFloorLoot = true;

		UBlueprintGeneratedClass* FloorLootClass = StaticFindObject<UBlueprintGeneratedClass>(nullptr, L"/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
		UBlueprintGeneratedClass* FloorLootWarmupClass = StaticFindObject<UBlueprintGeneratedClass>(nullptr, L"/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");

		TArray<AActor*> FloorLootActors, FloorLootWarmupActors;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), FloorLootClass, &FloorLootActors);
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), FloorLootWarmupClass, &FloorLootWarmupActors);

		for (AActor* InActor : FloorLootActors)
		{
			ABuildingContainer* FloorLoot = Cast<ABuildingContainer>(InActor);
			BuildingContainer::SpawnLoot(FloorLoot, nullptr, EFortPickupSourceTypeFlag::FloorLoot);

			InActor->K2_DestroyActor();
		}

		for (AActor* InActor : FloorLootWarmupActors)
		{
			ABuildingContainer* FloorLoot = Cast<ABuildingContainer>(InActor);
			BuildingContainer::SpawnLoot(FloorLoot, nullptr, EFortPickupSourceTypeFlag::FloorLoot);

			InActor->K2_DestroyActor();
		}

		FloorLootActors.Free();
		FloorLootWarmupActors.Free();
	}
#endif

}

uint8_t FortGameModeAthena::PickTeam(AFortGameModeAthena* Context, uint8_t PreferredTeam, AFortPlayerControllerAthena* Controller)
{
	static int CurrentTeam = 3;
	static int PlayersOnCurrentTeam = 0;

	AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(Context->GameState);
	UFortPlaylistAthena* Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;
	uint8_t ret = CurrentTeam;

	if (++PlayersOnCurrentTeam >= Playlist->MaxSquadSize)
	{
		CurrentTeam++;
		PlayersOnCurrentTeam = 0;
	}

	return ret;
}

void FortGameModeAthena::Patch()
{
	CreateHook(0x2BFFF70, ReadyToStartMatch, (void**)&oReadyToStartMatch);
	CreateHook(0xC407E0, SpawnDefaultPawnFor);
	CreateHook(0x2C03670, HandleStartingNewPlayer, (void**)&oHandleStartingNewPlayer);
	CreateHook(0xC35C10, PickTeam);
}
