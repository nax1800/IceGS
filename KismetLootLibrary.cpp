#include "pch.h"
#include "KismetLootLibrary.h"

#include <UObject/UObjectBase.h>
#include <UObject/UObjectGlobals.h>

int UKismetLootLibrary::GetClipSize(UFortWeaponRangedItemDefinition* Context)
{
	if (Context == nullptr || Context->IsA(UFortWeaponRangedItemDefinition::StaticClass()) == false)
		return -1;

	FFortRangedWeaponStats WeaponStats;
	if (UFortKismetLibrary::GetRangedWeaponStatsRow(Context->GetWeaponStatHandle(), &WeaponStats) == false)
		return -1;

	return WeaponStats.ClipSize;
}

float UKismetLootLibrary::RandomFloatForLoot(float AllWeightsSum)
{
	return (rand() * 0.000030518509f) * AllWeightsSum;
}

FFortLootTierData* UKismetLootLibrary::PickLootTierData(const std::vector<UDataTable*>& LTDTables, FName LootTierGroup, int ForcedLootTier, FName* OutRowName)
{
    float LootTier = ForcedLootTier;

    map<FName, FFortLootTierData*> TierGroupLTDs;

    CollectDataTablesRows<FFortLootTierData>(LTDTables, &TierGroupLTDs, [&](FName RowName, FFortLootTierData* TierData) -> bool {
        if (LootTierGroup == TierData->TierGroup)
        {
            if ((LootTier == -1 ? true : LootTier == TierData->LootTier))
            {
                return true;
            }
        }

        return false;
        });

    int Multiplier = LootTier == -1 ? 1 : LootTier;

    FFortLootTierData* ChosenRowLootTierData = PickWeightedElement<FName, FFortLootTierData*>(TierGroupLTDs,
        [](FFortLootTierData* LootTierData) -> float { return LootTierData->Weight; }, RandomFloatForLoot, -1,
        true, Multiplier, OutRowName);

    return ChosenRowLootTierData;
}

int UKismetLootLibrary::PickLevel(UFortWorldItemDefinition* ItemDefinition, int PreferredLevel)
{
    int MinLevel = ItemDefinition->MinLevel;
    int MaxLevel = ItemDefinition->MaxLevel;

    int PickedLevel = 0;

    if (PreferredLevel >= MinLevel)
        PickedLevel = PreferredLevel;

    if (MaxLevel >= 0)
    {
        if (PickedLevel <= MaxLevel)
            return PickedLevel;

        return MaxLevel;
    }

    return PickedLevel;
}

void UKismetLootLibrary::PickLootDropsFromLootPackage(const std::vector<UDataTable*>& LPTables, const FName& LootPackageName, std::vector<LootDrop>* OutEntries, int LootPackageCategory, int WorldLevel, bool bPrint, bool bCombineDrops)
{
    if (OutEntries == nullptr)
        return;

    map<FName, FFortLootPackageData*> LootPackageIDMap;

    CollectDataTablesRows<FFortLootPackageData>(LPTables, &LootPackageIDMap, [&](FName RowName, FFortLootPackageData* LootPackage) -> bool {
        if (LootPackage->LootPackageID != LootPackageName)
        {
            return false;
        }

        if (LootPackageCategory != -1 && LootPackage->LootPackageCategory != LootPackageCategory) // idk if proper
        {
            return false;
        }

        if (WorldLevel >= 0)
        {
            if (LootPackage->MaxWorldLevel >= 0 && WorldLevel > LootPackage->MaxWorldLevel)
                return 0;

            if (LootPackage->MinWorldLevel >= 0 && WorldLevel < LootPackage->MinWorldLevel)
                return 0;
        }

        return true;
        });

    if (LootPackageIDMap.size() == 0)
    {
        return;
    }

    FName PickedPackageRowName;
    FFortLootPackageData* PickedPackage = PickWeightedElement<FName, FFortLootPackageData*>(LootPackageIDMap,
        [](FFortLootPackageData* LootPackageData) -> float { return LootPackageData->Weight; }, RandomFloatForLoot,
        -1, true, 1, &PickedPackageRowName, bPrint);

    if (PickedPackage == nullptr)
        return;

    if (PickedPackage->LootPackageCall.Num() > 1)
    {
        if (PickedPackage->Count > 0)
        {
            int v9 = 0;

            while (v9 < PickedPackage->Count)
            {
                int LootPackageCategoryToUseForLPCall = 0; // hmm

                PickLootDropsFromLootPackage(LPTables,
                    PickedPackage->LootPackageCall.IsValid() ? UKismetStringLibrary::Conv_StringToName(PickedPackage->LootPackageCall) : FName(0),
                    OutEntries, LootPackageCategoryToUseForLPCall, WorldLevel, bPrint
                );

                v9++;
            }
        }

        return;
    }

    UFortItemDefinition* ItemDefinition = PickedPackage->ItemDefinition.Get();

    if (ItemDefinition == nullptr)
        return;

    UFortWeaponRangedItemDefinition* WeaponItemDefinition = Cast<UFortWeaponRangedItemDefinition>(ItemDefinition);
    int LoadedAmmo = WeaponItemDefinition ? GetClipSize(WeaponItemDefinition) : 0;

    UFortWorldItemDefinition* WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);

    if (WorldItemDefinition == nullptr)
        return;

    int CountMultiplier = 1;
    int FinalCount = CountMultiplier * PickedPackage->Count;

    if (FinalCount > 0)
    {
        int FinalItemLevel = 0;

        while (FinalCount > 0)
        {
            int MaxStackSize = ItemDefinition->MaxStackSize;

            int CurrentCountForEntry = MaxStackSize;

            if (FinalCount <= MaxStackSize)
                CurrentCountForEntry = FinalCount;

            if (CurrentCountForEntry <= 0)
                CurrentCountForEntry = 0;

            int ActualItemLevel = PickLevel(WorldItemDefinition, FinalItemLevel);

            bool bHasCombined = false;

            if (bCombineDrops)
            {
                for (LootDrop& CurrentLootDrop : *OutEntries)
                {
                    if (CurrentLootDrop->ItemDefinition == ItemDefinition)
                    {
                        int NewCount = CurrentLootDrop->Count + CurrentCountForEntry;

                        if (NewCount <= ItemDefinition->MaxStackSize)
                        {
                            bHasCombined = true;
                            CurrentLootDrop->Count = NewCount;
                        }
                    }
                }
            }

            if (!bHasCombined)
            {
                FFortItemEntry* NewItemEntry = new FFortItemEntry();
                NewItemEntry->ItemDefinition = ItemDefinition;
                NewItemEntry->Count = CurrentCountForEntry;
                NewItemEntry->LoadedAmmo = LoadedAmmo;
                NewItemEntry->Durability = 0x3F800000;
                NewItemEntry->Level = ActualItemLevel;

                OutEntries->push_back(LootDrop(NewItemEntry));
            }

            FinalCount -= CurrentCountForEntry;
        }
    }
}

float UKismetLootLibrary::GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumberLootDrops)
{
    if (LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num() || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryMaxArray.Num())
        return 0;


    float MinimumLootDrops = 0;

    if (LootTierData->LootPackageCategoryMinArray.Num() > 0)
    {
        for (int i = 0; i < LootTierData->LootPackageCategoryMinArray.Num(); ++i)
        {
            MinimumLootDrops += LootTierData->LootPackageCategoryMinArray[i];
        }
    }

    int SumLootPackageCategoryWeightArray = 0;

    if (LootTierData->LootPackageCategoryWeightArray.Num() > 0)
    {
        for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i)
        {
            if (LootTierData->LootPackageCategoryWeightArray[i] > 0)
            {
                auto LootPackageCategoryMaxArrayIt = LootTierData->LootPackageCategoryWeightArray[i];

                if (LootPackageCategoryMaxArrayIt < 0 || 0 < LootPackageCategoryMaxArrayIt)
                {
                    SumLootPackageCategoryWeightArray += LootTierData->LootPackageCategoryWeightArray[i];
                }
            }
        }
    }

    while (SumLootPackageCategoryWeightArray > 0)
    {
        float v29 = (float)rand() * 0.000030518509f;

        float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
            + (float)((float)SumLootPackageCategoryWeightArray * v29))
            + 0.5f) >> 1;

        MinimumLootDrops++;

        if (MinimumLootDrops >= OriginalNumberLootDrops)
            return MinimumLootDrops;

        SumLootPackageCategoryWeightArray--;
    }

    return MinimumLootDrops;
}

vector<LootDrop> UKismetLootLibrary::PickLootDrops(FName TierGroupName, int WorldLevel, int ForcedLootTier, bool bPrint, int recursive, bool bCombineDrops)
{
    std::vector<LootDrop> LootDrops;

    if (recursive > 6)
        return LootDrops;

    static std::vector<UDataTable*> LTDTables;
    static std::vector<UDataTable*> LPTables;

    static int LastNum1 = 14915;

    UFortPlaylistAthena* CurrentPlaylist = GetGameState()->CurrentPlaylistInfo.BasePlaylist;

    LTDTables.clear();
    LPTables.clear();

    bool bFoundPlaylistTable = false;

    if (CurrentPlaylist)
    {
        auto& LootTierDataSoft = CurrentPlaylist->LootTierData;
        auto& LootPackagesSoft = CurrentPlaylist->LootPackages;

        if (LootTierDataSoft.ObjectID.AssetPathName.ComparisonIndex && LootPackagesSoft.ObjectID.AssetPathName.ComparisonIndex)
        {
            auto LootTierDataStr = LootTierDataSoft.ObjectID.AssetPathName.ToString();
            auto LootPackagesStr = LootPackagesSoft.ObjectID.AssetPathName.ToString();
            auto LootTierDataTableIsComposite = LootTierDataStr.contains("Composite");
            auto LootPackageTableIsComposite = LootPackagesStr.contains("Composite");

            UDataTable* StrongLootTierData = nullptr;
            UDataTable* StrongLootPackage = nullptr;

            StrongLootTierData = LootTierDataSoft.Get();
            StrongLootPackage = LootPackagesSoft.Get();

            if (StrongLootTierData && StrongLootPackage)
            {
                LTDTables.push_back(StrongLootTierData);
                LPTables.push_back(StrongLootPackage);

                bFoundPlaylistTable = true;
            }
        }
    }

    if (bFoundPlaylistTable == false)
    {
        LTDTables.push_back(StaticFindObject<UDataTable>(nullptr, L"/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
        LPTables.push_back(StaticFindObject<UDataTable>(nullptr, L"/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
    }


    for (int i = 0; i < LTDTables.size(); ++i)
    {
        UDataTable* Table = LTDTables.at(i);

        if (UObjectBase::IsValidLowLevel(Table) == false)
            continue;
    }

    for (int i = 0; i < LPTables.size(); ++i)
    {
        UDataTable* Table = LPTables.at(i);

        if (UObjectBase::IsValidLowLevel(Table) == false)
            continue;
    }

    if (LTDTables.size() <= 0 || LPTables.size() <= 0)
        return LootDrops;

    FName LootTierRowName;
    FFortLootTierData* ChosenRowLootTierData = PickLootTierData(LTDTables, TierGroupName, ForcedLootTier, &LootTierRowName);

    if (ChosenRowLootTierData == nullptr)
        return LootDrops;

    float NumLootPackageDrops = ChosenRowLootTierData->NumLootPackageDrops;
    float NumberLootDrops = 0;

    if (NumLootPackageDrops > 0)
    {
        if (NumLootPackageDrops < 1)
        {
            NumberLootDrops = 1;
        }
        else
        {
            NumberLootDrops = (int)(float)((float)(NumLootPackageDrops + NumLootPackageDrops) - 0.5f) >> 1;
            float v20 = NumLootPackageDrops - NumberLootDrops;
            if (v20 > 0.0000099999997f)
            {
                NumberLootDrops += v20 >= (rand() * 0.000030518509f);
            }
        }
    }

    float AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(ChosenRowLootTierData, NumberLootDrops);

    LootDrops.reserve(AmountOfLootPackageDrops);

    if (AmountOfLootPackageDrops > 0)
    {
        for (int i = 0; i < AmountOfLootPackageDrops; ++i)
        {
            if (i >= ChosenRowLootTierData->LootPackageCategoryMinArray.Num())
                break;

            for (int j = 0; j < ChosenRowLootTierData->LootPackageCategoryMinArray[i]; ++j)
            {
                if (ChosenRowLootTierData->LootPackageCategoryMinArray[i] < 1)
                    break;

                int LootPackageCategory = i;

                PickLootDropsFromLootPackage(LPTables, ChosenRowLootTierData->LootPackage, &LootDrops, LootPackageCategory, WorldLevel, bPrint);
            }
        }
    }

    return LootDrops;
}
