#pragma once

class FortGameSession
{
public:
	static bool KickPlayer(AFortGameSession* Context, APlayerController* KickedPlayer, const FText& KickReason);

public:
	static void Patch();
};

