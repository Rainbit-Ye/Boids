// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAbilitySystemComponent.h"

URTAbilitySystemComponent::URTAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URTAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&URTAbilitySystemComponent::OnEffectAppliedToSelf);
}

void URTAbilitySystemComponent::OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer Tags;
	GameplayEffectSpec.GetAllAssetTags(Tags);
	OnGameplayEffectTags.Broadcast(Tags);
}


