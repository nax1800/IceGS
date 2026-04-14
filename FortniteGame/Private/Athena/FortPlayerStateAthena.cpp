#include "pch.h"
#include "Athena/FortPlayerStateAthena.h"

void (*FortPlayerStateAthena::ApplyCharacterCustomization)(AFortPlayerState*, AFortPlayerPawn*) = decltype(FortPlayerStateAthena::ApplyCharacterCustomization)(ImageBase + 0x13426B0);
