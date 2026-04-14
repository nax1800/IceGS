#include "pch.h"
#include "Engine/NetDriver.h"
#include "Engine/ReplicationDriver.h"

#include "Online/FortGameSession.h"

void (*NetDriver::SetWorld)(UNetDriver* Context, UWorld* InWorld) = decltype(NetDriver::SetWorld)(ImageBase + 0x28A21B0);

void (*ReplicationDriver::ServerReplicateActors)(UReplicationDriver* Context, float DeltaSeconds) = decltype(ReplicationDriver::ServerReplicateActors)(nullptr); // VFT + 0x56 =  ImageBase + 0x2A7140

void NetDriver::TickFlush(UNetDriver* Context, float DeltaSeconds)
{
	if (Context->ReplicationDriver)
		reinterpret_cast<void(*)(UReplicationDriver * Context, float DeltaSeconds)>(Context->ReplicationDriver->VTable[0x56])(Context->ReplicationDriver, DeltaSeconds);

	if (GetAsyncKeyState(VK_F5) & 1)
	{
		for (UNetConnection* ClientConnection : Context->ClientConnections)
		{
			if (ClientConnection == nullptr || ClientConnection->PlayerController == nullptr)
				continue;

			UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"Weapon.TryToFireRestrictedByTypeCooldowns 0", ClientConnection->PlayerController);
		}
	}

	return oTickFlush(Context, DeltaSeconds);
}

void NetDriver::Patch()
{
	CreateHook(0x28A3870, TickFlush, (void**)&oTickFlush);
}