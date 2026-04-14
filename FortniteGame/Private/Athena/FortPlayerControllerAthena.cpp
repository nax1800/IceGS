#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"

#include <FortKismetLibrary.h>
#include <Athena/FortGameModeAthena.h>
#include <Athena/FortPlayerStateAthena.h>
#include <Building/BuildingActor.h>
#include <Items/FortInventory.h>
#include <Items/FortPickup.h>

#include <AbilitySystemComponent.h>

#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include <UObject/UObjectGlobals.h>

#include <KismetBotLibrary.h>
#include <CheatHandler.h>


void FortPlayerControllerAthena::ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* P)
{
	Context->AcknowledgedPawn = P;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);

	P->CustomizationLoadout = Context->CustomizationLoadout;
	P->OnRep_CustomizationLoadout();

	PlayerState->HeroType = Context->CustomizationLoadout.Character->HeroDefinition;
	PlayerState->OnRep_HeroType();

	FortPlayerStateAthena::ApplyCharacterCustomization(PlayerState, P);
}

void FortPlayerControllerAthena::ServerReadyToStartMatch(AFortPlayerControllerAthena* Context)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	AbilitySystemComponent::ApplyAbilities(PlayerState->AbilitySystemComponent);

	Context->MatchReport = Cast<UAthenaPlayerMatchReport>(UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), Context));
	Context->MatchReport->bHasMatchStats = true;
	Context->MatchReport->bHasTeamStats = true;
	Context->MatchReport->bHasRewards = true;

	Context->RecordMatchStats();
	Context->RecordTeamStats();

	PlayerState->SquadId = (uint8)PlayerState->TeamIndex - 2;
	PlayerState->OnRep_PlayerTeam();
	PlayerState->OnRep_SquadId();

	PlayerState->WorldPlayerId = PlayerState->PlayerID;

	return oServerReadyToStartMatch(Context);
}

void FortPlayerControllerAthena::ServerExecuteInventoryItem(AFortPlayerControllerAthena* Context, FGuid& ItemGuid)
{
	if (Context->IsInAircraft() == true) 
		return;

	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Context->Pawn);
	if (Pawn == nullptr) 
		return;

	FFortItemEntry* FoundReplicatedEntry = FortInventory::FindItem(Context->WorldInventory, ItemGuid);
	if (FoundReplicatedEntry == nullptr) 
		return;

	UFortWeaponItemDefinition* ItemDefinition = Cast<UFortWeaponItemDefinition>(FoundReplicatedEntry->ItemDefinition);
	if (ItemDefinition == nullptr) 
		return;

	if (ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()) == true)
	{
		UFortDecoItemDefinition* DecoDefinition = Cast<UFortDecoItemDefinition>(ItemDefinition);
		if (DecoDefinition != nullptr)
		{
			Pawn->PickUpActor(nullptr, DecoDefinition);
			Pawn->CurrentWeapon->ItemEntryGuid = ItemGuid;

			if (AFortDecoTool_ContextTrap* ContextTrapTool = Cast<AFortDecoTool_ContextTrap>(Pawn->CurrentWeapon))
				ContextTrapTool->ContextTrapItemDefinition = Cast<UFortContextTrapItemDefinition>(ItemDefinition);

			ItemDefinition = nullptr;
		}
	}

	Pawn->EquipWeaponDefinition(ItemDefinition, ItemGuid);
}

void FortPlayerControllerAthena::ServerAttemptInventoryDrop(AFortPlayerControllerAthena* Context, FGuid ItemGuid, int32 Count)
{
	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Context->Pawn);
	if (Pawn == nullptr)
		return;

	FFortItemEntry* ReplicatedEntry = FortInventory::FindItem(Context->WorldInventory, ItemGuid);
	if (ReplicatedEntry == nullptr || Count > ReplicatedEntry->Count)
		return;

	AFortPickupAthena* SpawnedPickup = FortPickup::SpawnPickup(ReplicatedEntry->ItemDefinition, Count, ReplicatedEntry->LoadedAmmo, Pawn->K2_GetActorLocation(), false, Pawn);
	if (SpawnedPickup == nullptr)
		return;

	FortInventory::RemoveItem(Context->WorldInventory, ReplicatedEntry->ItemGuid, Count);
}

void FortPlayerControllerAthena::ServerCreateBuildingActor(AFortPlayerControllerAthena* Context, FBuildingClassData& BuildingClassData, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	TArray<ABuildingActor*> ExistingBuildings;
	EFortBuildPreviewMarkerOptionalAdjustment MarkerOptionalAdjustment;

	UWorld* World = Actor::GetWorld(Context);
	if (FortKismetLibrary::CanPlaceBuildableClassInStructuralGrid(World, BuildingClassData.BuildingClass.Get(), BuildLoc, BuildRot, bMirrored, ExistingBuildings, &MarkerOptionalAdjustment) == EFortStructuralGridQueryResults::CanAdd)
	{
		ABuildingSMActor* NewBuilding = BuildingActor::SpawnBuilding(World, BuildingClassData.BuildingClass.Get(), BuildLoc, BuildRot, World::CreateSpawnParams());
		NewBuilding->SetMirrored(bMirrored);
		NewBuilding->InitializeKismetSpawnedBuildingActor(NewBuilding, Context, true);
		NewBuilding->bPlayerPlaced = true;
		NewBuilding->Team = EFortTeam(PlayerState->TeamIndex);
		NewBuilding->OnRep_Team();
		
		if (Context->bBuildFree == false)
			FortInventory::RemoveItem(Context->WorldInventory, UFortKismetLibrary::K2_GetResourceItemDefinition(NewBuilding->ResourceType), 10);
	}
}

void FortPlayerControllerAthena::ServerBeginEditingBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToEdit)
{
	if (BuildingActorToEdit == nullptr)
		return;

	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Context->Pawn);
	if (Pawn == nullptr)
		return;

	static UFortEditToolItemDefinition* EditToolDef = StaticFindObject<UFortEditToolItemDefinition>(nullptr, L"/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
	if (Pawn->CurrentWeapon->WeaponData != EditToolDef)
	{
		FFortItemEntry Entry;

		for (FFortItemEntry& ReplicatedEntry : Context->WorldInventory->Inventory.ReplicatedEntries)
		{
			if (ReplicatedEntry.ItemDefinition == EditToolDef)
			{
				Entry = ReplicatedEntry;
			}
		}

		Context->ServerExecuteInventoryItem(Entry.ItemGuid);
	}

	AFortWeap_EditingTool* EditTool = Cast<AFortWeap_EditingTool>(Pawn->CurrentWeapon);
	EditTool->EditActor = BuildingActorToEdit;
	EditTool->OnRep_EditActor();

	BuildingActorToEdit->EditingPlayer = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	BuildingActorToEdit->OnRep_EditingPlayer();
}

void FortPlayerControllerAthena::ServerEditBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored)
{
	if (BuildingActorToEdit == nullptr || NewBuildingClass == nullptr)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	if (PlayerState == nullptr)
		return;

	FVector BuildLocation = BuildingActorToEdit->K2_GetActorLocation();
	FRotator BuildRotation = BuildingActorToEdit->K2_GetActorRotation();
	FVector BuildForwardVector = BuildingActorToEdit->GetActorForwardVector();
	FVector BuildRightVector = BuildingActorToEdit->GetActorRightVector();

	int yaw = round(float((int(BuildRotation.Yaw) + 360) % 360) / 10) * 10;

	if (BuildingActorToEdit->BuildingType != EFortBuildingType::Wall)
	{
		switch (RotationIterations)
		{
		case 1:
			BuildLocation = BuildLocation + BuildForwardVector * 256.0f + BuildRightVector * 256.0f;
			break;
		case 2:
			BuildLocation = BuildLocation + BuildRightVector * 512.0f;
			break;
		case 3:
			BuildLocation = BuildLocation + BuildForwardVector * -256.0f + BuildRightVector * 256.0f;
		}
	}

	BuildRotation.Yaw = yaw + 90 * RotationIterations;
	float HealthPercent = BuildingActorToEdit->GetHealthPercent();

	BuildingActorToEdit->SilentDie();

	ABuildingSMActor* NewBuildingActor = World::SpawnActor<ABuildingSMActor>(NewBuildingClass, BuildLocation, BuildRotation);
	if (NewBuildingActor == nullptr)
		return;

	if (BuildingActorToEdit->bIsInitiallyBuilding == false)
		NewBuildingActor->ForceBuildingHealth(NewBuildingActor->GetMaxHealth() * HealthPercent);

	NewBuildingActor->InitializeKismetSpawnedBuildingActor(NewBuildingActor, Context, true);
	NewBuildingActor->bPlayerPlaced = true;
	NewBuildingActor->Team = EFortTeam(PlayerState->TeamIndex);
	NewBuildingActor->OnRep_Team();

	if (NewBuildingActor->IsStructurallySupported() == false)
		NewBuildingActor->K2_DestroyActor();
}

void FortPlayerControllerAthena::ServerEndEditingBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToStopEditing)
{
	if (BuildingActorToStopEditing == nullptr)
		return;

	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Context->Pawn);
	if (Pawn == nullptr)
		return;

	BuildingActorToStopEditing->EditingPlayer = nullptr;
	BuildingActorToStopEditing->OnRep_EditingPlayer();

	AFortWeap_EditingTool* EditTool = Cast<AFortWeap_EditingTool>(Pawn->CurrentWeapon);
	if (EditTool == nullptr)
		return;

	EditTool->bEditConfirmed = true;
	EditTool->EditActor = nullptr;
	EditTool->OnRep_EditActor();
}

void FortPlayerControllerAthena::ServerTeleportToPlaygroundLobbyIsland(AFortPlayerControllerAthena* Context)
{
	AFortPlayerPawn* Pawn = Context->MyFortPawn;
	if (Pawn == nullptr)
		return;

	TArray<AActor*> AllCreativePlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPlayerStartCreative::StaticClass(), &AllCreativePlayerStarts);

	for (AActor* CurrentPlayerStart : AllCreativePlayerStarts)
	{
		bool bHasSpawnTag = Cast<AFortPlayerStartCreative>(CurrentPlayerStart)->PlayerStartTag.ToString() == "Playground.LobbyIsland.Spawn";
		if (bHasSpawnTag == false)
			continue;

		Pawn->K2_TeleportTo(CurrentPlayerStart->K2_GetActorLocation(), Pawn->K2_GetActorRotation());
		break;
	}

	AllCreativePlayerStarts.Free();
}

void FortPlayerControllerAthena::ServerPlayEmoteItem(AFortPlayerControllerAthena* Context, UFortMontageItemDefinitionBase* EmoteAsset)
{
	if (Context->MyFortPawn == nullptr  || Context->PlayerState  == nullptr || EmoteAsset == nullptr)
		return;

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	AFortPlayerPawn* MyFortPawn = Context->MyFortPawn;

	UFortAbilitySystemComponent* AbilitySystemComponent = PlayerState->AbilitySystemComponent;

	UObject* AbilityToUse = nullptr;

	if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass()))
	{
		static UBlueprintGeneratedClass* SprayAbilityClass = StaticFindObject<UBlueprintGeneratedClass>(nullptr, L"/Game/Abilities/Sprays/GAB_Spray_Generic.GAB_Spray_Generic_C");
		AbilityToUse = SprayAbilityClass->DefaultObject;
	}
	else if (EmoteAsset->IsA(UAthenaToyItemDefinition::StaticClass()))
	{
		UAthenaToyItemDefinition* ToyAsset = Cast<UAthenaToyItemDefinition>(EmoteAsset);
		AbilityToUse = ToyAsset->ToySpawnAbility->DefaultObject;
	}
	else if (EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass()))
	{
		UAthenaDanceItemDefinition* DanceAsset = Cast<UAthenaDanceItemDefinition>(EmoteAsset);
		MyFortPawn->bMovingEmote = DanceAsset->bMovingEmote;
		MyFortPawn->EmoteWalkSpeed = DanceAsset->WalkForwardSpeed;
		MyFortPawn->bMovingEmoteForwardOnly = DanceAsset->bMoveForwardOnly;

		UClass* CustomAbility = DanceAsset->CustomDanceAbility.Get();
		if (CustomAbility != nullptr)
		{
			AbilityToUse = CustomAbility->DefaultObject;
		}
		else
		{
			static UBlueprintGeneratedClass* EmoteAbilityClass = StaticFindObject<UBlueprintGeneratedClass>(nullptr, L"/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C");
			AbilityToUse = EmoteAbilityClass->DefaultObject;
		}
	}

	if (AbilityToUse)
	{
		FGameplayAbilitySpec NewSpec{};

		AbilitySystemComponent::SpecConstructor(&NewSpec, AbilityToUse, 1, -1, EmoteAsset);
		AbilitySystemComponent::GiveAbilityAndActivateOnce(AbilitySystemComponent, &NewSpec.Handle, NewSpec);
	}
}

void FortPlayerControllerAthena::ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* Context)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);

	if (PlayerState->RespawnData.bRespawnDataAvailable && PlayerState->RespawnData.bServerIsReady)
	{
		auto RespawnData = PlayerState->RespawnData;
		FTransform SpawnTransform{};

		FQuat Rotation{};
		FVector Scale = FVector(1, 1, 1);

		Rotation.X = PlayerState->RespawnData.RespawnRotation.Pitch;
		Rotation.Y = PlayerState->RespawnData.RespawnRotation.Roll;
		Rotation.Z = PlayerState->RespawnData.RespawnRotation.Yaw;
		
		SpawnTransform.Translation = PlayerState->RespawnData.RespawnLocation;
		SpawnTransform.Rotation = Rotation;
		SpawnTransform.Scale3D = Scale;

		APlayerPawn_Athena_C* NewPawn = Cast<APlayerPawn_Athena_C>(GetGameMode()->SpawnDefaultPawnAtTransform(Context, SpawnTransform));
		Context->Possess(NewPawn);
		Context->RespawnPlayerAfterDeath();

		NewPawn->SetHealth(100.f);
		NewPawn->SetShield(0.f);
	}

	PlayerState->RespawnData.bClientIsReady = true;
}

void FortPlayerControllerAthena::ClientOnPawnDied(AFortPlayerControllerAthena* Context, const FFortPlayerDeathReport& DeathReport)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	AFortPlayerPawn* MyFortPawn = Context->MyFortPawn;

#ifndef PLAYGROUND
	if (Context->WorldInventory && MyFortPawn)
	{
		for (FFortItemEntry& Entry : Context->WorldInventory->Inventory.ReplicatedEntries)
		{
			if (Cast<UFortWeaponItemDefinition>(Entry.ItemDefinition)->bCanBeDropped)
				FortPickup::SpawnPickup(Entry.ItemDefinition, Entry.Count, Entry.LoadedAmmo, MyFortPawn->K2_GetActorLocation());
		}
	}
#endif // !PLAYGROUND

	AFortPlayerStateAthena* KillerPlayerState = Cast<AFortPlayerStateAthena>(DeathReport.KillerPlayerState);
	AFortPlayerPawnAthena* KillerPawn = Cast<AFortPlayerPawnAthena>(DeathReport.KillerPawn);
	AFortPlayerControllerAthena* KillerPlayerController = KillerPlayerState ? Cast<AFortPlayerControllerAthena>(KillerPlayerState->Owner) : nullptr;

	PlayerState->PawnDeathLocation = MyFortPawn ? MyFortPawn->K2_GetActorLocation() : FVector();

	PlayerState->DeathInfo.bDBNO = MyFortPawn ? MyFortPawn->IsDBNO() : false;
	PlayerState->DeathInfo.DeathLocation = PlayerState->PawnDeathLocation;
	PlayerState->DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(DeathReport.Tags, PlayerState->DeathInfo.bDBNO);
	PlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : PlayerState;
	PlayerState->DeathInfo.Distance = MyFortPawn ? (PlayerState->DeathInfo.DeathCause != EDeathCause::FallDamage ? (KillerPawn ? KillerPawn->GetDistanceTo(MyFortPawn) : 0) : 0) : 0;
	PlayerState->DeathInfo.bInitialized = true;
	PlayerState->OnRep_DeathInfo();

	if (KillerPlayerState && KillerPawn && KillerPawn->Controller && KillerPawn->Controller != Context)
	{
		KillerPlayerState->KillScore++;
		KillerPlayerState->TeamKillScore++;
		KillerPlayerState->OnRep_Kills();
		KillerPlayerState->ClientReportKill(PlayerState);
		KillerPlayerState->ClientReportTeamKill(KillerPlayerState->TeamKillScore);
	}

	bool bRespawnAllowed = GetGameState()->IsRespawningAllowed(PlayerState);

	if (bRespawnAllowed == false && (MyFortPawn ? !MyFortPawn->IsDBNO() : true))
	{
		PlayerState->Place = GetGameState()->PlayersLeft;
		PlayerState->OnRep_Place();

		AFortWeapon* DamageCauser = nullptr;
		if (DeathReport.DamageCauser ? DeathReport.DamageCauser->IsA(AFortProjectileBase::StaticClass()) : false)
		{
			AActor* Owner = DeathReport.DamageCauser->Owner;

			if (Owner->IsA(AFortWeapon::StaticClass()))
			{
				DamageCauser = Cast<AFortWeapon>(Owner);
			}
			else if (Owner->IsA(AFortPlayerControllerAthena::StaticClass()))
			{
				AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(Owner);
				DamageCauser = Cast<AFortWeapon>(Controller->MyFortPawn->CurrentWeapon);
			}
			else if (Owner->IsA(AFortPlayerPawnAthena::StaticClass()))
			{
				AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(Owner);
				DamageCauser = Cast<AFortWeapon>(Pawn->CurrentWeapon);
			}
		}
		else if (AFortWeapon* Weapon = DeathReport.DamageCauser ? Cast<AFortWeapon>(DeathReport.DamageCauser) : nullptr)
			DamageCauser = Weapon;

		FortGameModeAthena::RemoveFromAlivePlayers(GetGameMode(), Context, KillerPlayerState == PlayerState ? nullptr : KillerPlayerState, KillerPawn, DamageCauser->WeaponData, PlayerState->DeathInfo.DeathCause, 0);
		Context->bMarkedAlive = false;

		if (MyFortPawn && KillerPlayerState && KillerPlayerState != PlayerState && KillerPlayerState->Place == 1)
		{
			UFortWeaponItemDefinition* KillerWeapon = DamageCauser ? DamageCauser->WeaponData : nullptr;

			KillerPlayerController->ClientNotifyWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);
			KillerPlayerController->ClientNotifyTeamWon(KillerPawn, KillerWeapon, PlayerState->DeathInfo.DeathCause);


			GetGameState()->WinningTeam = (uint32)KillerPlayerState->TeamIndex;
			GetGameState()->OnRep_WinningTeam();
			GetGameState()->WinningPlayerState = KillerPlayerState;
			GetGameState()->OnRep_WinningPlayerState();
		}
	}

	return oClientOnPawnDied(Context, DeathReport);
}

void FortPlayerControllerAthena::ServerCheat(AFortPlayerControllerAthena* Context, const FString& Msg)
{
	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Context->PlayerState);
	if (PlayerState == nullptr)
		return;

	APlayerPawn_Athena_C* Pawn = Cast<APlayerPawn_Athena_C>(Context->Pawn);
	if (Pawn == nullptr)
		return;

	string PlayerName = PlayerState->GetPlayerName().ToString();


	if (Msg.IsValid() == false || Msg.Num() <= 0)
		return;

	std::vector<std::string> Arguments;
	auto Message = Msg.ToString();

	size_t start = Message.find('\\');

	int zz = 0;

	while (Message.find(" ") != std::string::npos)
	{
		auto arg = Message.substr(0, Message.find(' '));
		Arguments.push_back(arg);
		Message.erase(0, Message.find(' ') + 1);
		zz++;
	}

	Arguments.push_back(Message);
	zz++;

	auto NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	if (Arguments.size() >= 1)
		CheatHandler::HandleCheat(Context, Arguments);
}

void FortPlayerControllerAthena::GetPlayerViewPoint(AFortPlayerControllerAthena* Context, FVector& out_Location, FRotator& out_Rotation)
{
	if (Context->StateName == FName(322))
	{
		out_Location = Context->LastSpectatorSyncLocation;
		out_Rotation = Context->LastSpectatorSyncRotation;
	}
	else if (APawn* Pawn = Context->Pawn)
	{
		out_Location = Pawn->K2_GetActorLocation();
		out_Rotation = Context->GetControlRotation();
		return;
	}

	return oGetPlayerViewPoint(Context, out_Location, out_Rotation);
}

void FortPlayerControllerAthena::Patch()
{
	VirtualHook<AFortPlayerControllerAthena>(0x105, ServerAcknowledgePossession);
	VirtualHook<AFortPlayerControllerAthena>(0x254, ServerReadyToStartMatch, (void**)&oServerReadyToStartMatch);

	VirtualHook<AFortPlayerControllerAthena>(0x1F6, ServerExecuteInventoryItem);
	VirtualHook<AFortPlayerControllerAthena>(0x208, ServerAttemptInventoryDrop);

	VirtualHook<AFortPlayerControllerAthena>(0x214, ServerCreateBuildingActor);
	VirtualHook<AFortPlayerControllerAthena>(0x21A, ServerBeginEditingBuildingActor);
	VirtualHook<AFortPlayerControllerAthena>(0x216, ServerEditBuildingActor);
	VirtualHook<AFortPlayerControllerAthena>(0x218, ServerEndEditingBuildingActor);

	VirtualHook<AFortPlayerControllerAthena>(0x43A, ServerTeleportToPlaygroundLobbyIsland);

	VirtualHook<AFortPlayerControllerAthena>(0x1BA, ServerPlayEmoteItem);

	VirtualHook<AFortPlayerControllerAthena>(0x43D, ServerClientIsReadyToRespawn);

	VirtualHook<AFortPlayerControllerAthena>(0x1B8, ServerCheat);

	CreateHook(0x16781A0, ClientOnPawnDied, (void**)&oClientOnPawnDied);
	CreateHook(0x12C9360, GetPlayerViewPoint, (void**)&oGetPlayerViewPoint);
}
