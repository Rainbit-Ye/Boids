// Fill out your copyright notice in the Description page of Project Settings.

#include "BoidsFunction.h"
#include "MyDemo/Boids/FishBVHSubsystem.h"
#include "Engine/World.h"

FVector UBoidsFunction::ComputeObstacleAvoidance(
	const FVector& Pos,
	const FVector& Forward,
	const FGuid& EntityID,
	float AvoidRadius,
	ECollisionChannel PhysChannel,
	const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	if (!BVH || BVH->GetEntityCount() == 0)
	{
		return Forward;
	}

	// === Step 1: 检测前方是否有障碍 ===
	const FVector AheadPoint = Pos + Forward * AvoidRadius;
	FVector ObstaclePos;
	const bool bObstacleAhead = BVH->HasNeighborInRadius(AheadPoint, AvoidRadius * 0.8f, EntityID, PhysChannel, ObstaclePos);

	if (!bObstacleAhead)
	{
		return Forward;
	}

	const FVector ToObstacle = (ObstaclePos - Pos).GetSafeNormal();

	// === Step 2: 同心环均匀采样搜寻 XZ 平面的无障碍通行点 ===
	const float SearchStep = AvoidRadius * 0.4f;
	const int32 MaxRings = 6;
	const float ClearRadius = AvoidRadius * 0.5f;

	const float ForwardAzimuth = FMath::Atan2(Forward.Z, Forward.X);
	const float PerFishPhase = static_cast<float>((EntityID.A ^ EntityID.B ^ EntityID.C ^ EntityID.D) & 0xFFFF) / 0xFFFF * UE_PI;

	FVector BestDir = Forward;
	bool bFound = false;

	for (int32 ring = 1; ring <= MaxRings && !bFound; ++ring)
	{
		const float Radius = SearchStep * ring;
		const int32 NumPoints = FMath::Max(4, ring * 8);
		const float AngleStep = 2.0f * UE_PI / NumPoints;

		for (int32 p = 0; p < NumPoints; ++p)
		{
			const int32 SignedIndex = (p % 2 == 0) ? (p / 2) : -(p + 1) / 2;
			const float Angle = ForwardAzimuth + UE_PI * 0.5f + AngleStep * SignedIndex + PerFishPhase;
			const FVector TestPoint(
				ObstaclePos.X + Radius * FMath::Cos(Angle),
				ObstaclePos.Y,
				ObstaclePos.Z + Radius * FMath::Sin(Angle)
			);

			if (!BVH->HasNeighborInRadius(TestPoint, ClearRadius, EntityID, PhysChannel))
			{
				BestDir = (TestPoint - Pos).GetSafeNormal();
				bFound = true;
				break;
			}
		}
	}

	if (!bFound)
	{
		BestDir = -ToObstacle;
	}

	return BestDir;
}
