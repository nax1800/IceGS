#include "pch.h"
#include "Abilities/GameplayAbility.h"

bool GameplayAbility::CanActivateAbility(UGameplayAbility* Context, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags)
{
	return true;
}


void GameplayAbility::Patch()
{
	CreateHook(0x660100, CanActivateAbility);
}