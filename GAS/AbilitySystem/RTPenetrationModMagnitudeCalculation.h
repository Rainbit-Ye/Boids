// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "RTPenetrationModMagnitudeCalculation.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API URTPenetrationModMagnitudeCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	URTPenetrationModMagnitudeCalculation();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition AttackDef;
};
