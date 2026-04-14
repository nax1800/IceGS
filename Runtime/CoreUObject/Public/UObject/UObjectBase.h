#pragma once
class UObjectBase
{
public:
	static bool (*IsValidLowLevel)(UObject* Context);
};

