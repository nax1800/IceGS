#include "pch.h"
#include "UObject/UObjectBase.h"


bool (*UObjectBase::IsValidLowLevel)(UObject* Context) = decltype(UObjectBase::IsValidLowLevel)(ImageBase + 0x1A00920);

