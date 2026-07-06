// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyDemo/GAS/AbilitySystem/EffectActor/RTEffectActorBase.h"
#include "EffectBox.generated.h"

UCLASS()
class MYDEMO_API AEffectBox : public ARTEffectActorBase
{
	GENERATED_BODY()

public:
	AEffectBox();
	
protected:
	virtual void OnOverlap(AActor* OtherActor) override;
	virtual void OnEndOverlap(AActor* OtherActor) override;
public:
};
