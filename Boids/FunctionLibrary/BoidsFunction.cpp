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

FVector UBoidsFunction::ComputeAlignment(
	const FVector& Pos,
	const FVector& Forward,
	const FGuid& EntityID,
	float AlignRadius,
	int32 MaxNeighbors,
	float Weight,
	const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	if (!BVH || BVH->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	BVH->QueryKNN(Pos, AlignRadius, MaxNeighbors, EntityID, Neighbors);

	if (Neighbors.Num() == 0)
	{
		return Forward;
	}

	// 邻居平均方向
	FVector AvgDir = FVector::ZeroVector;
	for (const FKNNResult& N : Neighbors)
	{
		AvgDir += N.ForwardDir.GetSafeNormal();
	}
	AvgDir /= static_cast<float>(Neighbors.Num());
	AvgDir.Normalize();

	// 自身方向 + 邻居平均方向加权混合
	return (Forward * (1.0f - Weight) + AvgDir * Weight).GetSafeNormal();
}

FVector UBoidsFunction::ComputeCohesion(
	const FVector& Pos,
	const FVector& Forward,
	const FGuid& EntityID,
	float CohesionRadius,
	int32 MaxNeighbors,
	float Weight,
	float MaxTurnAngle,
	const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	if (!BVH || BVH->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	BVH->QueryKNN(Pos, CohesionRadius, MaxNeighbors, EntityID, Neighbors);

	if (Neighbors.Num() == 0)
	{
		return Forward;
	}

	// 计算邻居位置中心
	FVector CenterOfMass = FVector::ZeroVector;
	for (const FKNNResult& N : Neighbors)
	{
		CenterOfMass += N.Position;
	}
	CenterOfMass /= static_cast<float>(Neighbors.Num());

	// 朝中心的方向
	const FVector ToCenter = (CenterOfMass - Pos);
	const float DistToCenter = ToCenter.Size();
	if (DistToCenter < 1.0f)
	{
		return Forward;  // 已经在中心
	}
	const FVector DirToCenter = ToCenter / DistToCenter;

	// 当前朝向到中心的夹角
	const float DotFC = FVector::DotProduct(Forward, DirToCenter);
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotFC, -1.0f, 1.0f)));

	// 实际转角 = min(角度 * 权重, 最大转角)
	const float TurnAngle = FMath::Min(Angle * Weight, MaxTurnAngle);

	if (TurnAngle < 0.1f)
	{
		return Forward;
	}

	// 绕垂直轴旋转 Forward 朝向中心方向
	const FVector Cross = FVector::CrossProduct(Forward, DirToCenter);
	const FVector Axis = Cross.IsNearlyZero() ? FVector::UpVector : Cross.GetSafeNormal();

	return Forward.RotateAngleAxis(TurnAngle, Axis);
}

FVector UBoidsFunction::ComputeSeparation(
	const FVector& Pos,
	const FVector& Forward,
	const FGuid& EntityID,
	float SepRadius,
	int32 MaxNeighbors,
	float Strength,
	const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	if (!BVH || BVH->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	BVH->QueryKNN(Pos, SepRadius, MaxNeighbors, EntityID, Neighbors);

	if (Neighbors.Num() == 0)
	{
		return Forward;
	}

	// 累加排斥力：越近的邻居推力越大
	FVector Repulsion = FVector::ZeroVector;
	for (const FKNNResult& N : Neighbors)
	{
		const FVector ToNeighbor = N.Position - Pos;
		const float Dist = ToNeighbor.Size();
		if (Dist < 1.0f)
		{
			// 完全重叠 → 任意方向推开
			Repulsion += Forward * -1.0f;
			continue;
		}

		const float T = FMath::Clamp(Dist / SepRadius, 0.0f, 1.0f);  // 越近 → 0，越远 → 1
		const float Force = 1.0f - T;                                  // 越近越强
		Repulsion -= ToNeighbor.GetSafeNormal() * Force;
	}

	if (Repulsion.IsNearlyZero())
	{
		return Forward;
	}

	Repulsion.Normalize();

	// 和当前朝向按 Strength 加权混合
	return (Forward * (1.0f - Strength) + Repulsion * Strength).GetSafeNormal();
}
