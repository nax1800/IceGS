#include "pch.h"
#include "GameFramework/Actor.h"

UWorld* Actor::GetWorld(AActor* Context)
{
	return reinterpret_cast<UWorld * (*)(AActor*)>(Context->VTable[0x28])(Context);
}
