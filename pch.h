#ifndef PCH_H
#define PCH_H

#include <Windows.h>
#include <iostream>
#include <format>
#include <thread>
#include <map>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cstdint>

#include "MinHook/include/MinHook.h"
#include "CppSDK/SDK.hpp"

using namespace SDK;
using namespace std;

//#define NOMCP
#define DEV
#define MATCHMAKING
//#define CREATIVE
#define CHRISTMASBUS

#ifndef CREATIVE
#define PLAYGROUND
#endif

inline uint64_t ImageBase = *(uint64_t*)(__readgsqword(0x60) + 0x10);
inline vector<uintptr_t> OffsetsToNull = { 0x239B850, 0xF0AE20, 0x12EC920 };

inline void CreateHook(uintptr_t InOffset, LPVOID InDetour, LPVOID* InOriginal = nullptr)
{
	MH_CreateHook((LPVOID)(ImageBase + InOffset), InDetour, InOriginal);
}

inline void VirtualHook(UObject* Object, int Index, void* Detour, void** OG = nullptr)
{
	if (Object->VTable == nullptr || Object->VTable[Index] == nullptr)
		return;

	if (OG != nullptr) 
		*OG = Object->VTable[Index];

	DWORD oldProtection;
	VirtualProtect(&Object->VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);
	Object->VTable[Index] = Detour;
	VirtualProtect(&Object->VTable[Index], 8, oldProtection, NULL);
}

template<typename T = UObject*>
inline void VirtualHook(uint64 Index, void* Detour, void** OG = nullptr)
{
	VirtualHook(T::GetDefaultObj(), Index, Detour, OG);
}

static void* nullOriginal = nullptr;

template <typename T = void*>
inline void ExecHook(UFunction* Context, void* Detour, T& OG = nullOriginal)
{
	if (Context == nullptr)
		return;

	if (is_same_v<T, void*> == false)
		OG = reinterpret_cast<T>(Context->ExecFunction);

	Context->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(Detour);
}

template<typename T>
inline T* Cast(void* InArg)
{
	return reinterpret_cast<T*>(InArg);
}

inline void PatchByte(uintptr_t Offset, uintptr_t Byte)
{
	uint8_t* func = (uint8_t*)(ImageBase + Offset);
	DWORD dwProtection;
	VirtualProtect((PVOID)func, 1, PAGE_EXECUTE_READWRITE, &dwProtection);
	*func = Byte;
	DWORD dwTemp;
	VirtualProtect((PVOID)func, 1, dwProtection, &dwTemp);
}

#ifdef MATCHMAKING
inline void EnableMatchmaking()
{
	PatchByte(0x2B952BE, 0x74);
}
#endif

inline AFortGameModeAthena* GetGameMode()
{
	return Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
}

inline AFortGameStateAthena* GetGameState()
{
	return Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
}

inline void SendMessageToConsole(AFortPlayerController* Context, FString Msg)
{
	static FName TypeName = UKismetStringLibrary::Conv_StringToName(L"Event");
	Context->ClientMessage(Msg, TypeName, 1);
}

inline float EvaluateCurveTable(FScalableFloat Context, float i = 0.f)
{
	if (Context.Curve.CurveTable == nullptr || Context.Curve.RowName.ComparisonIndex > 0)
		return Context.Value;

	float Out = 0.f;
	UDataTableFunctionLibrary::EvaluateCurveTableRow(Context.Curve.CurveTable, Context.Curve.RowName, i, nullptr, &Out, FString());
	return Context.Value * Out;
}

#endif //PCH_H
