#pragma once

class ReplicationDriver
{
public:
	static void (*ServerReplicateActors)(UReplicationDriver* Context, float DeltaSeconds);
};