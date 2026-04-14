#pragma once

class GameplayAbility
{
public:
	static bool CanActivateAbility(UGameplayAbility* Context, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags);

	static void Patch();
};

