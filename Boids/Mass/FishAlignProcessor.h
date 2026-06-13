// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "FishAlignProcessor.generated.h"

UCLASS()
class MYDEMO_API UFishAlignProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UFishAlignProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery BoidsQuery;
};
