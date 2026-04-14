#include "pch.h"
#include "Online/FortGameSession.h"

bool FortGameSession::KickPlayer(AFortGameSession* Context, APlayerController* KickedPlayer, const FText& KickReason)
{
	return true;
}

void FortGameSession::Patch()
{
	return CreateHook(0x2784E50, KickPlayer);
}