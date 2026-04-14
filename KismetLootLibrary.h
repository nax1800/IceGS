#pragma once
#include <UObject/UObjectBase.h>

struct LootDrop
{
    FFortItemEntry* ItemEntry;

    FFortItemEntry* operator->() {
        return ItemEntry;
    }

    ~LootDrop()
    {

    }
};

class UKismetLootLibrary
{
public:
    static int GetClipSize(UFortWeaponRangedItemDefinition* Context);
    static float RandomFloatForLoot(float AllWeightsSum);

    template <typename KeyType, typename ValueType>
    static inline ValueType PickWeightedElement(const std::map<KeyType, ValueType>& Elements,
        std::function<float(ValueType)> GetWeightFn,
        std::function<float(float)> RandomFloatGenerator = RandomFloatForLoot,
        float TotalWeightParam = -1, bool bCheckIfWeightIsZero = false, int RandMultiplier = 1, KeyType* OutName = nullptr, bool bPrint = false, bool bKeepGoingUntilWeGetValue = false)
    {
        float TotalWeight = TotalWeightParam;

        if (TotalWeight == -1)
        {
            TotalWeight = std::accumulate(Elements.begin(), Elements.end(), 0.0f, [&](float acc, const std::pair<KeyType, ValueType>& p) {
                auto Weight = GetWeightFn(p.second);
                return acc + Weight;
                });
        }

        float RandomNumber = RandMultiplier * RandomFloatGenerator(TotalWeight);

        for (auto& Element : Elements)
        {
            float Weight = GetWeightFn(Element.second);

            if (bCheckIfWeightIsZero && Weight == 0)
                continue;

            if (RandomNumber <= Weight)
            {
                if (OutName)
                    *OutName = Element.first;

                return Element.second;
            }

            RandomNumber -= Weight;
        }

        if (bKeepGoingUntilWeGetValue)
            return PickWeightedElement<KeyType, ValueType>(Elements, GetWeightFn, RandomFloatGenerator, TotalWeightParam, bCheckIfWeightIsZero, RandMultiplier, OutName, bPrint, bKeepGoingUntilWeGetValue);

        return ValueType();
    }

    template <typename RowStructType = uint8>
    static inline void CollectDataTablesRows(const std::vector<UDataTable*>& DataTables, map<FName, RowStructType*>* OutMap, std::function<bool(FName, RowStructType*)> Check = []() { return true; })
    {
        vector<UDataTable*> DataTablesToIterate;

        for (UDataTable* DataTable : DataTables)
        {
            if (!UObjectBase::IsValidLowLevel(DataTable))
            {
                continue;
            }

            DataTablesToIterate.push_back(DataTable);
        }

        for (auto CurrentDataTable : DataTablesToIterate)
        {
            for (TPair<FName, uint8_t*>& CurrentPair : CurrentDataTable->RowMap)
            {
                if (Check(CurrentPair.Key(), (RowStructType*)CurrentPair.Value()))
                {
                    (*OutMap)[CurrentPair.Key()] = (RowStructType*)CurrentPair.Value();
                }
            }
        }
    }

    static FFortLootTierData* PickLootTierData(const std::vector<UDataTable*>& LTDTables, FName LootTierGroup, int ForcedLootTier = -1, FName* OutRowName = nullptr);
    static int PickLevel(UFortWorldItemDefinition* ItemDefinition, int PreferredLevel);
    static void PickLootDropsFromLootPackage(const std::vector<UDataTable*>& LPTables, const FName& LootPackageName, std::vector<LootDrop>* OutEntries, int LootPackageCategory = -1, int WorldLevel = 0, bool bPrint = false, bool bCombineDrops = true);
    static float GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumberLootDrops);
    static vector<LootDrop> PickLootDrops(FName TierGroupName, int WorldLevel, int ForcedLootTier = -1, bool bPrint = false, int recursive = 0, bool bCombineDrops = true);
};

