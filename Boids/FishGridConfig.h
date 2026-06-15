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

	UPROPERTY(EditAnywhere, Category = "Grid")
	float AlignWeight = .3f;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	float CohesionWeight = .3f;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	float SeparationWeight = .1f;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	float AvoidWeight = .3f;

	
};
