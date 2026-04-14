#pragma once

struct FFrame;

class BuildingFoundation
{
private:
	static inline void (*oSetDynamicFoundationEnabled)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*oSetDynamicFoundationTransform)(UObject* Context, FFrame& Stack, void* Ret);

public:
	static void ShowFoundation(ABuildingFoundation* Context);
	static void SetFoundationTransform(ABuildingFoundation* Context, const FTransform& Transform);
	static void SetDynamicFoundationEnabled(UObject* Context, FFrame& Stack, void* Ret);
	static void SetDynamicFoundationTransform(UObject* Context, FFrame& Stack, void* Ret);

	static void Patch();
};

