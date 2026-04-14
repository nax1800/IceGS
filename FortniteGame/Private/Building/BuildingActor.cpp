#include "pch.h"
#include "Building/BuildingActor.h"
#include <Items/FortInventory.h>
#include <UObject/UObjectGlobals.h>

ABuildingSMActor* (*BuildingActor::SpawnBuilding)(UWorld* World, UClass* Class, const FVector& Location, FRotator& Rotation, const FActorSpawnParameters& SpawnParameters) = decltype(BuildingActor::SpawnBuilding)(ImageBase + 0x28127E0);

void BuildingActor::OnDamageServer(ABuildingActor* Context, float Damage, const FGameplayTagContainer& DamageTags, const FVector& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext)
{
	oOnDamageServer(Context, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    if (Context == nullptr || DamageCauser == nullptr || InstigatedBy == nullptr)
        return;

	if (Context->IsA(ABuildingSMActor::StaticClass()) == false)
        return;

    ABuildingSMActor* ContextAsSM = Cast<ABuildingSMActor>(Context);

    if (DamageCauser->IsA(AFortWeapon::StaticClass()) == false)
        return;

    AFortWeapon* Weapon = Cast<AFortWeapon>(DamageCauser);
    if (Weapon->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) == false)
        return;

    if (Context->bDestroyed == true || Context->bPlayerPlaced == true)
        return;

    AFortPlayerControllerAthena* PlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
    if (PlayerController == nullptr)
        return;

    bool bWeakSpotHit = Damage == 100.f;

    UFortResourceItemDefinition* ResourceDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(ContextAsSM->ResourceType);
    if (ResourceDefinition == nullptr)
        return;

    FCurveTableRowHandle BuildingResourceAmountOverride = ContextAsSM->BuildingResourceAmountOverride;

    int ResourceAmount = 0;

    if (BuildingResourceAmountOverride.RowName.ComparisonIndex)
    {
        UCurveTable* ResourceRates = GetGameState()->CurrentPlaylistInfo.BasePlaylist->ResourceRates.Get();

        if (ResourceRates == nullptr)
            ResourceRates = StaticFindObject<UCurveTable>(nullptr, L"/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");

        float AmountOut = 0;

        UDataTableFunctionLibrary::EvaluateCurveTableRow(ResourceRates, BuildingResourceAmountOverride.RowName, 0.f, nullptr, &AmountOut, L"");

        float Amount = AmountOut / (Context->GetMaxHealth() / Damage);
        ResourceAmount = round(Amount);
    }

    PlayerController->ClientReportDamagedResourceBuilding(ContextAsSM, ContextAsSM->ResourceType, ResourceAmount, Context->bDestroyed, bWeakSpotHit);

    if (ResourceAmount > 0)
        FortInventory::AddItem(PlayerController->WorldInventory, ResourceDefinition, ResourceAmount);
}

void BuildingActor::Patch()
{
	CreateHook(0x14C5540, OnDamageServer, (void**)&oOnDamageServer);
}
