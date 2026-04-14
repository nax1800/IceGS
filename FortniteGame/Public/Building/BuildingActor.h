#pragma once
#include <Engine/World.h>

class BuildingActor
{
private:
	static inline void (*oOnDamageServer)(ABuildingActor* Context, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext);

public:
	static ABuildingSMActor* (*SpawnBuilding)(UWorld* World, UClass* Class, const FVector& Location, FRotator& Rotation, const FActorSpawnParameters& SpawnParameters);

	static void OnDamageServer(ABuildingActor* Context, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext);

	static void Patch();
};

