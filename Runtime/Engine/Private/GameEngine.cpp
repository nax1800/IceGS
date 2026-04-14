#include "pch.h"
#include "Engine/Engine.h"

UNetDriver* (*Engine::CreateNetDriver)(UEngine*, UWorld*, FName) = decltype(Engine::CreateNetDriver)(ImageBase + 0x2B261C0);