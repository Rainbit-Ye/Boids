// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoidsFunction.generated.h"

struct FKNNResult;

UCLASS()
class MYDEMO_API UBoidsFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 一次性完成对齐+凝聚+分离，内部只做一次 KNN 查询
	 * @param OutAlign		输出：对齐方向
	 * @param OutCohesion	输出：凝聚方向
	 * @param OutSeparation	输出：分离方向
	 */
	UFUNCTION(BlueprintCallable)
	static void ComputeAllBoidsForces(
		const UObject* WorldContextObject,
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float AlignRadius, int32 AlignMaxNeighbors, float AlignWeight,
		float CohesionRadius, int32 CohesionMaxNeighbors, float CohesionWeight, float CohesionMaxTurnAngle,
		float SepRadius, int32 SepMaxNeighbors, float SepStrength,
		FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation);

	/** C++ 专用重载：复用外部 TArray，避免每鱼分配 */
	static void ComputeAllBoidsForces(
		const UObject* WorldContextObject,
		const FVector& Pos, const FVector& Forward, const FGuid& EntityID,
		float AlignRadius, int32 AlignMaxNeighbors, float AlignWeight,
		float CohesionRadius, int32 CohesionMaxNeighbors, float CohesionWeight, float CohesionMaxTurnAngle,
		float SepRadius, int32 SepMaxNeighbors, float SepStrength,
		FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation,
		TArray<FKNNResult>& ScratchNeighbors);

	UFUNCTION(BlueprintCallable)
	static FVector ComputeAlignment(const UObject* WorldContextObject, float Weight, const FGuid& EntityID, const FVector& Forward, int32 MaxNeighbors, const
	                                FVector& Pos, float AlignRadius);
	
	UFUNCTION(BlueprintCallable)
	static FVector ComputeCohesion(
		const UObject* WorldContextObject,
		const FVector& Forward,
		const FGuid& EntityID,
		float CohesionRadius,
		int32 MaxNeighbors,
		float Weight,
		float MaxTurnAngle,
		const FVector& Pos
	);

	UFUNCTION(BlueprintCallable)
	static FVector ComputeSeparation(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float SepRadius,
		int32 MaxNeighbors,
		float Strength,
		const UObject* WorldContextObject
	);

	/**
	 * 锥形视野避障：以 Forward 为中轴生成采样向量 → ID哈希打乱 → Sweep → 最小偏角安全方向
	 */
	static FVector ComputeObstacleAvoidanceCone(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float ProbeDistance,
		float AgentRadius,
		const TArray<float>& SampleAngles,
		int32 AzimuthSamples,
		int32 MaxSteps,
		float SmoothFactor,
		ECollisionChannel Channel,
		const UObject* WorldContextObject
	);

	/** C++ 专用重载：复用外部 TArray<FVector> ScratchSampleDirs，避免每鱼分配 */
	static FVector ComputeObstacleAvoidanceCone(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float ProbeDistance,
		float AgentRadius,
		const TArray<float>& SampleAngles,
		int32 AzimuthSamples,
		int32 MaxSteps,
		float SmoothFactor,
		ECollisionChannel Channel,
		const UObject* WorldContextObject,
		TArray<FVector>& ScratchSampleDirs);
};
