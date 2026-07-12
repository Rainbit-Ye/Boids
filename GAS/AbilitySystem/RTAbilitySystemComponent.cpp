// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAbilitySystemComponent.h"

#include "MyDemo/GAS/RTGameplayTags.h"

URTAbilitySystemComponent::URTAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URTAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&URTAbilitySystemComponent::OnEffectAppliedToSelf);

	const FRTGameplayTags& GameplayTags = FRTGameplayTags::Get();
	UE_LOG(LogTemp,Warning,TEXT("PenetrationTag: %s"),*GameplayTags.PenetrationTag.ToString());
}

void URTAbilitySystemComponent::OnEffectAppliedToSelf(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& GameplayEffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer Tags;
	GameplayEffectSpec.GetAllAssetTags(Tags);
	OnGameplayEffectTags.Broadcast(Tags);
}


