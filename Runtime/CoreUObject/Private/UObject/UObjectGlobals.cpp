#include "pch.h"
#include "UObject/UObjectGlobals.h"

UObject* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass) 
{ 
	static auto Func = reinterpret_cast<UObject * (*)(UClass*, void*, const wchar_t*, bool)>(InSDKUtils::GetImageBase() + 0x1A0A330);
	return Func(Class, InOuter, Name, ExactClass);
}

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32 LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation) 
{
	static auto Func = reinterpret_cast<UObject * (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool)>(InSDKUtils::GetImageBase() + 0x1A0ABF0);
	return Func(Class, InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation);
}