#pragma once
class UObjectBaseUtility
{
public:
	static void* (*GetInterfaceAddress)(UObject* Context, UClass* InterfaceClass);
};

