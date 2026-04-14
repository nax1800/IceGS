#pragma once

class Engine
{
public:
	static UNetDriver* (*CreateNetDriver)(UEngine* Context, UWorld* InWorld, FName NetDriverDefinition);
};

