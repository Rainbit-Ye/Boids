// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "FishGridProcessor.generated.h"

/**
 * 网格更新处理器：每帧收集所有鱼实体位置，重建均匀网格供查询
 */
UCLASS()
class MYDEMO_API UFishGridProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UFishGridProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery BVHQuery;
};
