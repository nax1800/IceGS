#pragma once

struct FFrame;

class FortAthenaCreativePortal
{
public:
	static void TeleportPlayerToLinkedVolume(UObject* Context, FFrame& Stack);
	static void TeleportPlayer(UObject* Context, FFrame& Stack);

	static void Patch();
};

