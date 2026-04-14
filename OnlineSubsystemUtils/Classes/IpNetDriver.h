#pragma once

class IpNetDriver
{
public:
	static bool (*InitListen)(UNetDriver* Context, UWorld* InWorld, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);
};

