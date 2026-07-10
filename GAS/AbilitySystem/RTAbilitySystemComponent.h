// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RTAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEffectAppliedToSelf, const FGameplayTagContainer& GameplayTagContainer);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYDEMO_API URTAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	FOnEffectAppliedToSelf OnGameplayEffectTags;
public:
	// Sets default values for this component's properties
	URTAbilitySystemComponent();
	
	void AbilityActorInfoSet();
protected:
	
	void OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
public:
};
