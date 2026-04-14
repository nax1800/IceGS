#pragma once
class FortWorldItem
{
public:
	static bool SetLoadedAmmo(UFortWorldItem* Context, int InCount);

	static void Patch();
};

