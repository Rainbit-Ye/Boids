// Fill out your copyright notice in the Description page of Project Settings.


#include "RTPenetrationModMagnitudeCalculation.h"

#include "RTAttributeSet.h"
#include "MyDemo/GAS/Interface/CombatInterface.h"

URTPenetrationModMagnitudeCalculation::URTPenetrationModMagnitudeCalculation()
{
	AttackDef.AttributeToCapture = URTAttributeSet::GetAttackAttribute();
	AttackDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	AttackDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(AttackDef);
}

float URTPenetrationModMagnitudeCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = SourceTags;
	EvalParams.TargetTags = TargetTags;

	float Attack = 0.0f;
	GetCapturedAttributeMagnitude(AttackDef,Spec,EvalParams,Attack);
	Attack = FMath::Max<float>(Attack,0.0f);

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	float Level = 1;
	if (CombatInterface)
	{
		Level = CombatInterface->GetCharacterLevel();
	}
	
	return Attack + (Level * 1.5) + 10;
}
