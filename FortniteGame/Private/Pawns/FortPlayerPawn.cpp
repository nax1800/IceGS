#include "pch.h"
#include "Pawns/FortPlayerPawn.h"

#include <UObject/Stack.h>

#include <Items/FortInventory.h>

void (*FortPlayerPawn::OnRep_ZiplineState)(AFortPlayerPawn* Context) = decltype(FortPlayerPawn::OnRep_ZiplineState)(ImageBase + 0x1286C50);

void FortPlayerPawn::ServerHandlePickup(AFortPlayerPawn* Context, AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
	if (Pickup == nullptr || Pickup->bPickedUp == true) 
		return;

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Context->Controller);
	if (PlayerController == nullptr) 
		return;

	Context->IncomingPickups.Add(Pickup);
	FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
	PickupLocationData.StartDirection = (FVector_NetQuantizeNormal)(InStartDirection);
	PickupLocationData.FlyTime = 0.40f;
	PickupLocationData.PickupTarget = Context;
	PickupLocationData.ItemOwner = Context;
	PickupLocationData.bPlayPickupSound = bPlayPickupSound;
	PickupLocationData.PickupGuid = Context->CurrentWeapon ? Context->CurrentWeapon->ItemEntryGuid : FGuid();
	Pickup->OnRep_PickupLocationData();

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void FortPlayerPawn::ServerHandlePickupWithSwap(AFortPlayerPawn* Context, AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
	if (Pickup == nullptr || Pickup->bPickedUp == true)
		return;

	AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(Context->GetController());
	if (Controller == nullptr)
		return;

	Context->IncomingPickups.Add(Pickup);

	FFortPickupLocationData& PickupLocationData = Pickup->PickupLocationData;
	PickupLocationData.StartDirection = (FVector_NetQuantizeNormal)(InStartDirection);
	PickupLocationData.PickupTarget = Context;
	PickupLocationData.FlyTime = 0.40f;
	PickupLocationData.ItemOwner = Context;
	PickupLocationData.bPlayPickupSound = bPlayPickupSound;
	PickupLocationData.PickupGuid = Swap;
	Pickup->OnRep_PickupLocationData();

	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void FortPlayerPawn::NetMulticast_Athena_BatchedDamageCues(AFortPlayerPawn* Context, const FAthenaBatchedDamageGameplayCues_Shared& SharedData, const FAthenaBatchedDamageGameplayCues_NonShared& NonSharedData)
{
	oNetMulticast_Athena_BatchedDamageCues(Context, SharedData, NonSharedData);

	AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(Context->Controller);
	if (PlayerController == nullptr) 
		return;

	AFortWeapon* CurrentWeapon = Context->CurrentWeapon;
	if (CurrentWeapon)
	{
		FFortItemEntry* CurrentItemEntry = FortInventory::FindItem(PlayerController->WorldInventory, CurrentWeapon->ItemEntryGuid);

		for (UFortWorldItem* ItemInstance : PlayerController->WorldInventory->Inventory.ItemInstances)
		{
			if (UKismetGuidLibrary::EqualEqual_GuidGuid(ItemInstance->ItemEntry.ItemGuid, CurrentWeapon->ItemEntryGuid))
			{
				ItemInstance->ItemEntry.LoadedAmmo = CurrentWeapon->AmmoCount;
				PlayerController->WorldInventory->Inventory.MarkArrayDirty();
				break;
			}
		}

		if (CurrentItemEntry)
		{
			CurrentItemEntry->LoadedAmmo = CurrentWeapon->AmmoCount;
			PlayerController->WorldInventory->Inventory.MarkItemDirty(*CurrentItemEntry);
			PlayerController->WorldInventory->Inventory.MarkArrayDirty();
		}
	}
}

void FortPlayerPawn::OnCapsuleBeginOverlap(AFortPlayerPawn* Context, UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Context == nullptr || OtherActor == nullptr)
		return oOnCapsuleBeginOverlap(Context, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (Context->IsDBNO() == true)
		return oOnCapsuleBeginOverlap(Context, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (OtherActor->IsA(AFortPickupAthena::StaticClass()) == true)
	{
		AFortPickup* Pickup = Cast<AFortPickup>(OtherActor);
		if (Pickup->PawnWhoDroppedPickup != Context)
		{
			UFortItemDefinition* ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;

			if (ItemDefinition == nullptr)
				return;

			if (FortInventory::GetQuickBars(ItemDefinition) != EFortQuickBars::Primary)
				Context->ServerHandlePickup(Pickup, 0.4f, FVector(), true);
		}
	}

	return oOnCapsuleBeginOverlap(Context, OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void FortPlayerPawn::ServerSendZiplineState(AFortPlayerPawn* Context, const FZiplinePawnState& InZiplineState)
{
	FZiplinePawnState& ZiplineState = Context->ZiplineState;
	if (InZiplineState.AuthoritativeValue > ZiplineState.AuthoritativeValue)
	{
		if (!ZiplineState.bIsZiplining)
		{
			if (ZiplineState.bJumped)
			{
				float ZiplineJumpDampening = -0.5f;
				float ZiplineJumpStrength = 1500.f;

				FVector& Velocity = Context->CharacterMovement->Velocity;

				FVector LaunchVelocity = FVector(-750.0f, -750.0f, 1500.0f);

				if (ZiplineJumpDampening * Velocity.X >= -750.f)
					LaunchVelocity.X = fminf(-0.5f * Velocity.X, 750.0f);

				if (ZiplineJumpDampening * Velocity.Y >= -750.f)
					LaunchVelocity.Y = fminf(-0.5f * Velocity.Y, 750.0f);

				Context->LaunchCharacter(LaunchVelocity, false, false);
			}
		}
	}

	OnRep_ZiplineState(Context);
}

void FortPlayerPawn::MovingEmoteStopped(UObject* Context, FFrame& Stack)
{
	AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context);
	if (Pawn == nullptr)
		return;

	Pawn->bMovingEmote = false;
	Pawn->bMovingEmoteForwardOnly = false;
}

void FortPlayerPawn::Patch()
{
	//CreateHook(0x164D450, NetMulticast_Athena_BatchedDamageCues, (void**)&oNetMulticast_Athena_BatchedDamageCues); // THIS CAUSES RELOADING TO BE BUGGED
	CreateHook(0x128D660, OnCapsuleBeginOverlap, (void**)&oOnCapsuleBeginOverlap);

	ExecHook(AFortPlayerPawn::StaticClass()->GetFunction("FortPlayerPawn", "MovingEmoteStopped"), MovingEmoteStopped);

	VirtualHook<APlayerPawn_Athena_C>(0x1B1, ServerHandlePickup);
	VirtualHook<APlayerPawn_Athena_C>(0x1AF, ServerHandlePickupWithSwap);
	VirtualHook<APlayerPawn_Athena_C>(0x1BA, ServerSendZiplineState);
}
