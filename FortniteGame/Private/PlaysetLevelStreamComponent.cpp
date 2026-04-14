#include "pch.h"
#include "PlaysetLevelStreamComponent.h"

void (*PlaysetLevelStreamComponent::LoadPlayset)(UPlaysetLevelStreamComponent*) = decltype(PlaysetLevelStreamComponent::LoadPlayset)(ImageBase + 0x1364050);