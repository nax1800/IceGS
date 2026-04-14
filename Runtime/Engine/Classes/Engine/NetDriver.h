#pragma once
class NetDriver
{
private:
	static inline void (*oTickFlush)(UNetDriver* Context, float DeltaSeconds);

public:
	static void (*SetWorld)(UNetDriver* Context, UWorld* InWorld);

	static void TickFlush(UNetDriver* Context, float DeltaSeconds);

	static void Patch();
};

