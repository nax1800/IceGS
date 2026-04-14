#pragma once

struct FFrame;

class FortPlayerPawn
{
private:
	static inline void (*oNetMulticast_Athena_BatchedDamageCues)(AFortPlayerPawn* Context, const FAthenaBatchedDamageGameplayCues_Shared& SharedData, const FAthenaBatchedDamageGameplayCues_NonShared& NonSharedData);
	static inline void (*oOnCapsuleBeginOverlap)(AFortPlayerPawn* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	static void (*OnRep_ZiplineState)(AFortPlayerPawn* Context);

	static void ServerHandlePickup(AFortPlayerPawn* Context, AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);
	static void ServerHandlePickupWithSwap(AFortPlayerPawn* Context, AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);
	static void NetMulticast_Athena_BatchedDamageCues(AFortPlayerPawn* Context, const FAthenaBatchedDamageGameplayCues_Shared& SharedData, const FAthenaBatchedDamageGameplayCues_NonShared& NonSharedData);
	static void OnCapsuleBeginOverlap(AFortPlayerPawn* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	static void ServerSendZiplineState(AFortPlayerPawn* Context, const FZiplinePawnState& InZiplineState);
	static void MovingEmoteStopped(UObject* Context, FFrame& Stack);

	static void Patch();
};

