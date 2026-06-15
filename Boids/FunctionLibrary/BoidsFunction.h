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
	static void ComputeAllBoidsForces(
		const UObject* WorldContextObject,
		const FVector& Pos, const FVector& Forward, const FGuid& EntityID,
		float AlignRadius, int32 AlignMaxNeighbors, float AlignWeight,
		float CohesionRadius, int32 CohesionMaxNeighbors, float CohesionWeight, float CohesionMaxTurnAngle,
		float SepRadius, int32 SepMaxNeighbors, float SepStrength,
		FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation,
		TArray<FKNNResult>& ScratchNeighbors);
	
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
