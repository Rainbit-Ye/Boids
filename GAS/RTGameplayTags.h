// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 
 */
struct FRTGameplayTags
{
public:
	static const FRTGameplayTags& Get(){return GameplayTags;}
	static void InitNativeGameplayTags();
	
	FGameplayTag HealthTag;
	FGameplayTag MaxHealthTag;

	FGameplayTag ManaTag;
	FGameplayTag MaxManaTag;
	
	FGameplayTag AttackTag;
	FGameplayTag DefenceTag;
	FGameplayTag CriticalRateTag;
	FGameplayTag CriticalHitTag;
		
	FGameplayTag PenetrationTag;
	FGameplayTag LifeStealTag;
protected:
private:
	static FRTGameplayTags GameplayTags;
};
