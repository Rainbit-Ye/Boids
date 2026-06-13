// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "FishInitProcessor.generated.h"

/**
 * 鱼实体初始化处理器：设置随机朝向、移动参数、唯一ID
 */
UCLASS()
class MYDEMO_API UFishInitProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
public:
	UFishInitProcessor();

protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery FishInitQuery;
};
