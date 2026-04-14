#include "pch.h"
#include "AbilitySystemComponent.h"

#include "UObject/UObjectGlobals.h"

bool (*AbilitySystemComponent::InternalTryActivateAbility)(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, FGameplayEventData* TriggerEventData) = decltype(AbilitySystemComponent::InternalTryActivateAbility)(ImageBase + 0x677F70);
FGameplayAbilitySpecHandle* (*AbilitySystemComponent::GiveAbility)(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle* outHandle, FGameplayAbilitySpec inSpec) = decltype(AbilitySystemComponent::GiveAbility)(ImageBase + 0x676730);
FGameplayAbilitySpecHandle* (*AbilitySystemComponent::GiveAbilityAndActivateOnce)(void*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) = decltype(AbilitySystemComponent::GiveAbilityAndActivateOnce)(ImageBase + 0x676850);
__int64 (*AbilitySystemComponent::SpecConstructor)(FGameplayAbilitySpec* Context, UObject* Ability, int Level, int InputID, UObject* SourceObject) = decltype(AbilitySystemComponent::SpecConstructor)(ImageBase + 0x69D0F0);

FGameplayAbilitySpec* AbilitySystemComponent::FindAbilitySpecFromHandle(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle)
{
    for (int i = 0; i < Context->ActivatableAbilities.Items.Num(); i++)
    {
        if (Context->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle)
        {
            return &Context->ActivatableAbilities.Items[i];
        }
    }

    return nullptr;
}

void AbilitySystemComponent::InternalServerTryActivateAbility(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Context, Handle);
    if (!Spec)
    {
        Context->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }

    const UGameplayAbility* AbilityToActivate = Spec->Ability;

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (InternalTryActivateAbility(Context, Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
    }
    else
    {
        Context->ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
        Context->ActivatableAbilities.MarkItemDirty(*Spec);
    }
}

FGameplayAbilitySpec* AbilitySystemComponent::FindGameplayAbility(UAbilitySystemComponent* Context, UGameplayAbility* GameplayAbility)
{
    for (int i = 0; i < Context->ActivatableAbilities.Items.Num(); i++)
    {
        FGameplayAbilitySpec& Spec = Context->ActivatableAbilities.Items[i];
        if (Spec.Ability == nullptr) 
            continue;

        if (Spec.Ability == GameplayAbility) 
            return &Spec;
    }

    return nullptr;
}

void AbilitySystemComponent::RemoveGameplayAbility(UAbilitySystemComponent* Context, UGameplayAbility* GameplayAbility)
{
    if (GameplayAbility == nullptr)
        return;

    FGameplayAbilitySpec* AbilitySpec = FindGameplayAbility(Context, GameplayAbility);

    if (AbilitySpec == nullptr) 
        return;

    Context->ClientCancelAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
    Context->ClientEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo);
    Context->ServerEndAbility(AbilitySpec->Handle, AbilitySpec->ActivationInfo, {});
}

void AbilitySystemComponent::GrantGameplayAbility(UAbilitySystemComponent* Context, UClass* GameplayAbilityClass)
{
    FGameplayAbilitySpec NewSpec{};

    SpecConstructor(&NewSpec, GameplayAbilityClass->DefaultObject, 1, -1, nullptr);
    GiveAbility(Context, &NewSpec.Handle, NewSpec);
}

void AbilitySystemComponent::ApplyAbilities(UAbilitySystemComponent* Context)
{
    static auto AbilitySet = StaticFindObject<UFortAbilitySet>(nullptr, L"/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_DefaultPlayer.GAS_DefaultPlayer");
    TArray<TSubclassOf<UFortGameplayAbility>> GameplayAbilities = AbilitySet->GameplayAbilities;
    for (TSubclassOf<UFortGameplayAbility> GameplayAbility : GameplayAbilities)
    { 
        GrantGameplayAbility(Context, GameplayAbility.Get());
    }
}

void AbilitySystemComponent::Patch()
{
    VirtualHook<UFortAbilitySystemComponentAthena>(0xF4, InternalServerTryActivateAbility);
}