// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "MyPlayerState.generated.h"
/**
 * 
 */
UCLASS()
class MYDEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	UAbilitySystemComponent* AbilitySystemComponent;
	FGameplayTag tag;
	FGameplayAttribute Attribute;
	UGameplayAbility* Ability;

private:
};
