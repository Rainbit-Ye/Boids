// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FishGridConfig.generated.h"

/**
 * 网格配置 DataAsset：包围盒 + 格子大小
 * 资产位置：/Game/DataTable/DA_GridConfig
 */
UCLASS(BlueprintType)
class MYDEMO_API UFishGridConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Grid")
	FVector BoundMin = FVector(-5000, -5000, -2000);

	UPROPERTY(EditAnywhere, Category = "Grid")
	FVector BoundMax = FVector(5000, 5000, 2000);

	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize = 600.f;

	UPROPERTY(EditAnywhere, Category = "Boids", meta = (DisplayName = "对齐权重（匹配邻居方向）"))
	float AlignWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Boids", meta = (DisplayName = "凝聚权重（向邻居中心靠拢）"))
	float CohesionWeight = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Boids", meta = (DisplayName = "分离权重（推开重叠的鱼）"))
	float SeparationWeight = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Boids", meta = (DisplayName = "惯性权重（保持当前朝向）"))
	float AvoidWeight = 0.15f;

	
};
