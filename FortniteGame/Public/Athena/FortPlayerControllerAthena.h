#pragma once
class FortPlayerControllerAthena
{
private:
	static inline void (*oServerReadyToStartMatch)(AFortPlayerControllerAthena* Context);
	static inline void (*oClientOnPawnDied)(AFortPlayerControllerAthena* Context, const FFortPlayerDeathReport& DeathReport);
	static inline void (*oGetPlayerViewPoint)(AFortPlayerControllerAthena* Context, FVector& Loc, FRotator& Rot);

public:
	static void ServerAcknowledgePossession(AFortPlayerControllerAthena* Context, APlayerPawn_Athena_C* P);
	static void ServerReadyToStartMatch(AFortPlayerControllerAthena* Context);

	static void ServerExecuteInventoryItem(AFortPlayerControllerAthena* Context, FGuid& ItemGuid);
	static void ServerAttemptInventoryDrop(AFortPlayerControllerAthena* Context, FGuid ItemGuid, int32 Count);

	static void ServerCreateBuildingActor(AFortPlayerControllerAthena* Context, FBuildingClassData& BuildingClassData, FVector_NetQuantize10& BuildLoc, FRotator& BuildRot, bool bMirrored);
	static void ServerBeginEditingBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToEdit);
	static void ServerEditBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored);
	static void ServerEndEditingBuildingActor(AFortPlayerControllerAthena* Context, ABuildingSMActor* BuildingActorToStopEditing);

	static void ServerTeleportToPlaygroundLobbyIsland(AFortPlayerControllerAthena* Context);

	static void ServerPlayEmoteItem(AFortPlayerControllerAthena* Context, UFortMontageItemDefinitionBase* EmoteAsset);

	static void ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* Context);

	static void ClientOnPawnDied(AFortPlayerControllerAthena* Context, const FFortPlayerDeathReport& DeathReport);

	static void ServerCheat(AFortPlayerControllerAthena* Context, const FString& Msg);

	static void GetPlayerViewPoint(AFortPlayerControllerAthena* Context, FVector& Loc, FRotator& Rot);

	static void Patch();
};

