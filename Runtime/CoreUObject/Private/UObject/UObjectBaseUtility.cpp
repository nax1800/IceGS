#include "pch.h"
#include "UObject/UObjectBaseUtility.h"

void* (*UObjectBaseUtility::GetInterfaceAddress)(UObject* Context, UClass* InterfaceClass) = decltype(UObjectBaseUtility::GetInterfaceAddress)(ImageBase + 0x19FDE70);
