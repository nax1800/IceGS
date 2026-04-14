#include "pch.h"
#include "IpNetDriver.h"

bool (*IpNetDriver::InitListen)(UNetDriver* Context, UWorld* InWorld, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error) = decltype(IpNetDriver::InitListen)(ImageBase + 0x446AF0);
