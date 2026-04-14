#pragma once

#define	INVALID_OBJECT	(UObject*)-1

UObject* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass = false);
UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename = NULL, uint32 LoadFlags = 0, UPackageMap* Sandbox = NULL, bool bAllowObjectReconciliation = true);

template<typename T>
T* StaticFindObject(UObject* InOuter, const TCHAR* Name, bool ExactClass = false)
{
	return Cast<T>(StaticFindObject(T::StaticClass(), InOuter, Name, ExactClass));
}

template<typename T>
T* StaticLoadObject(UObject* InOuter, const TCHAR* Name, const TCHAR* Filename = NULL, uint32 LoadFlags = 0, UPackageMap* Sandbox = NULL, bool bAllowObjectReconciliation = true)
{
	return Cast<T>(StaticLoadObject(T::StaticClass(), InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation));
}

void CollectGarbage(EObjectFlags KeepFlags, bool bPerformFullPurge = true);

class GarbageCollection
{
public:
	static void Patch();
};
