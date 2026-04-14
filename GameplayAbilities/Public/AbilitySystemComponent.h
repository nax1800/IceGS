#pragma once
class AbilitySystemComponent
{
public:
	static bool (*InternalTryActivateAbility)(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, FGameplayEventData* TriggerEventData);
	static FGameplayAbilitySpecHandle* (*GiveAbility)(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle* outHandle, FGameplayAbilitySpec inSpec);
	static FGameplayAbilitySpecHandle* (*GiveAbilityAndActivateOnce)(void*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec);
	static  __int64 (*SpecConstructor)(FGameplayAbilitySpec* Context, UObject* Ability, int Level, int InputID, UObject* SourceObject);

	static FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle);
	static void InternalServerTryActivateAbility(UAbilitySystemComponent* Context, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, FGameplayEventData* TriggerEventData);

	static FGameplayAbilitySpec* FindGameplayAbility(UAbilitySystemComponent* Context, UGameplayAbility* GameplayAbility);
	static void RemoveGameplayAbility(UAbilitySystemComponent* Context, UGameplayAbility* GameplayAbility);
	static void GrantGameplayAbility(UAbilitySystemComponent* Context, UClass* GameplayAbilityClass);
	static void ApplyAbilities(UAbilitySystemComponent* Context);

	static void Patch();
};