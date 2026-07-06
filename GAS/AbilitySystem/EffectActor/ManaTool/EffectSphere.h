// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EffectBox.h"
#include "EffectSphere.generated.h"

UCLASS()
class MYDEMO_API AEffectSphere : public ARTEffectActorBase
{
	GENERATED_BODY()

public:
	AEffectSphere();
	
	virtual void OnOverlap(AActor* OtherActor) override;
	virtual void OnEndOverlap(AActor* OtherActor) override;
};
