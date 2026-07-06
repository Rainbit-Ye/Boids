// Fill out your copyright notice in the Description page of Project Settings.


#include "RTPlayerState.h"

#include "AbilitySystem/RTAbilitySystemComponent.h"
#include "AbilitySystem/RTAttributeSet.h"

ARTPlayerState::ARTPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);// 保证可以复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<URTAttributeSet>(TEXT("AttributeSet"));
	
	NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* ARTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
