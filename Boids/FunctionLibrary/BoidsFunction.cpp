// Fill out your copyright notice in the Description page of Project Settings.

#include "BoidsFunction.h"
#include "MyDemo/Boids/FishGridSubsystem.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

// ID 哈希打乱数组（Fisher-Yates shuffle with entity-id-based seed）
static void HashShuffle(TArray<FVector>& Array, uint32 Seed)
{
	const int32 N = Array.Num();
	for (int32 i = N - 1; i > 0; --i)
	{
		const uint32 H = Seed + static_cast<uint32>(i) * 2654435761u;
		const int32 j = static_cast<int32>(H % static_cast<uint32>(i + 1));
		Array.Swap(i, j);
	}
}

static FVector AlignFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Forward,
	int32 MaxNeighbors)
{
	FVector AvgDir = FVector::ZeroVector;
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (Count >= MaxNeighbors) break;
		AvgDir += N.ForwardDir.GetSafeNormal();
		Count++;
	}
	if (Count == 0) return Forward;
	return (AvgDir / static_cast<float>(Count)).GetSafeNormal();
}

static FVector CohesionFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Pos,
	const FVector& Forward, int32 MaxNeighbors)
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (Count >= MaxNeighbors) break;
		CenterOfMass += N.Position;
		Count++;
	}
	if (Count == 0) return Forward;
	CenterOfMass /= static_cast<float>(Count);

	const FVector ToCenter = CenterOfMass - Pos;
	const float DistToCenter = ToCenter.Size();
	if (DistToCenter < 1.f) return Forward;

	return ToCenter / DistToCenter;
}

static FVector SeparationFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Pos,
	const FVector& Forward, int32 MaxNeighbors)
{
	FVector Repulsion = FVector::ZeroVector;
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (Count >= MaxNeighbors) break;
		Count++;

		const FVector ToNeighbor = N.Position - Pos;
		const float DistSq = N.DistanceSq;
		if (DistSq < 1.f)
		{
			Repulsion += Forward * -1.f;
			continue;
		}

		// 反比于距离：定向排斥量 = -Dir / Dist，越近推力越大
		const FVector ToNeighborDir = ToNeighbor / FMath::Sqrt(DistSq);
		Repulsion -= ToNeighborDir / FMath::Sqrt(DistSq);
	}
	if (Repulsion.IsNearlyZero()) return Forward;
	return Repulsion.GetSafeNormal();
}

// ==================== 合并接口：一次 KNN 完成对齐+凝聚+分离 ====================

void UBoidsFunction::ComputeAllBoidsForces(
	const UObject* WorldContextObject,
	const FVector& Pos, const FVector& Forward, const FGuid& EntityID,
	int32 AlignMaxNeighbors,
	int32 CohesionMaxNeighbors,
	int32 SepMaxNeighbors,
	FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation,
	TArray<FKNNResult>& ScratchNeighbors)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;

	OutAlign     = Forward;
	OutCohesion  = Forward;
	OutSeparation = Forward;

	if (!Grid || Grid->GetEntityCount() <= 1)
	{
		return;
	}

	const int32 MaxNeighbors = FMath::Max3(AlignMaxNeighbors, CohesionMaxNeighbors, SepMaxNeighbors);

	Grid->QueryKNN(Pos, MaxNeighbors, EntityID, ScratchNeighbors);
	if (ScratchNeighbors.Num() == 0)
	{
		return;
	}

	// 各行为返回纯目标方向（不再内部混合 Forward），由 Processor 统一加权
	OutAlign     = AlignFromNeighbors(ScratchNeighbors, Forward, AlignMaxNeighbors);
	OutCohesion  = CohesionFromNeighbors(ScratchNeighbors, Pos, Forward, CohesionMaxNeighbors);
	OutSeparation = SeparationFromNeighbors(ScratchNeighbors, Pos, Forward, SepMaxNeighbors);
}


FVector UBoidsFunction::ComputeObstacleAvoidanceCone(
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
	TArray<FVector>& ScratchSampleDirs)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return Forward;
	}

	const FVector Fwd = Forward.GetSafeNormal();

	// ---- 构建局部坐标系（以 Forward 为中轴） ----
	const FVector UpRef = FMath::Abs(Fwd.Z) < 0.99f ? FVector::UpVector : FVector::RightVector;
	const FVector Right = FVector::CrossProduct(Fwd, UpRef).GetSafeNormal();
	const FVector Up   = FVector::CrossProduct(Right, Fwd).GetSafeNormal();

	// ---- 1. 快速检测正前方是否畅通（球体重叠，不扫掠） ----
	{
		FCollisionShape Sphere = FCollisionShape::MakeSphere(AgentRadius);
		FCollisionQueryParams Params;
		Params.bTraceComplex = false;
		const FVector CheckPos = Pos + Fwd * ProbeDistance;
		if (!World->OverlapAnyTestByChannel(CheckPos, FQuat::Identity, Channel, Sphere, Params))
		{
			return Forward; // 正前方畅通，无需避障
		}
	}

	// ---- 2. 生成锥形采样向量数组（复用外部 ScratchSampleDirs） ----
	ScratchSampleDirs.Reset();
	ScratchSampleDirs.Reserve(SampleAngles.Num() * AzimuthSamples + 1);

	// 正前方优先
	ScratchSampleDirs.Add(Fwd);

	for (float HalfAngleDeg : SampleAngles)
	{
		if (HalfAngleDeg <= 0.f) continue;
		const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);

		if (HalfAngleDeg >= 179.f)
		{
			// 180° → 全方向采样（球面覆盖，避免盲区）
			const int32 FullSamples = FMath::Max(AzimuthSamples, 12);
			for (int32 i = 0; i < FullSamples; ++i)
			{
				const float Theta = (static_cast<float>(i) / FullSamples) * UE_PI * 2.f;
				const float Phi = (static_cast<float>(i % 4) / 4.f - 0.375f) * UE_PI;
				FVector LocalDir;
				LocalDir.X = FMath::Cos(Theta) * FMath::Cos(Phi);
				LocalDir.Y = FMath::Sin(Theta) * FMath::Cos(Phi);
				LocalDir.Z = FMath::Sin(Phi);
				FVector WorldDir = Right * LocalDir.X + Fwd * LocalDir.Y + Up * LocalDir.Z;
				WorldDir.Normalize();
				if (!WorldDir.IsNearlyZero())
				{
					ScratchSampleDirs.Add(WorldDir);
				}
			}
		}
		else
		{
			// 锥形采样：绕 Forward 均匀分布方位
			for (int32 a = 0; a < AzimuthSamples; ++a)
			{
				const float Azimuth = (static_cast<float>(a) / AzimuthSamples) * UE_PI * 2.f;
				FVector LocalDir;
				LocalDir.X = FMath::Cos(Azimuth) * FMath::Sin(HalfAngleRad);   // Right
				LocalDir.Y = FMath::Cos(HalfAngleRad);                           // Forward
				LocalDir.Z = FMath::Sin(Azimuth) * FMath::Sin(HalfAngleRad);   // Up
				FVector WorldDir = Right * LocalDir.X + Fwd * LocalDir.Y + Up * LocalDir.Z;
				WorldDir.Normalize();
				if (!WorldDir.IsNearlyZero())
				{
					ScratchSampleDirs.Add(WorldDir);
				}
			}
		}
	}

	// ---- 3. EntityID 哈希打乱，避免集群鱼全部挤向同一绕行侧 ----
	{
		const uint32 Seed = static_cast<uint32>(
			EntityID.A ^ EntityID.B ^ EntityID.C ^ EntityID.D);
		HashShuffle(ScratchSampleDirs, Seed);
	}

	// ---- 4. 逐方向球体重叠检测，找最小偏角安全方向 ----
	FVector BestDir = Fwd;
	float BestAngle = 180.f;
	bool bFoundSafe = false;

	FCollisionQueryParams OverlapParams;
	OverlapParams.bTraceComplex = false;

	const int32 Steps = FMath::Min(MaxSteps, ScratchSampleDirs.Num());
	for (int32 i = 0; i < Steps; ++i)
	{
		const FVector TestDir = ScratchSampleDirs[i];
		const FVector CheckPos = Pos + TestDir * ProbeDistance;
		FCollisionShape OverlapShape = FCollisionShape::MakeSphere(AgentRadius);

		const bool bBlocked = World->OverlapAnyTestByChannel(
			CheckPos, FQuat::Identity, Channel, OverlapShape, OverlapParams);

		if (!bBlocked)
		{
			const float Dot = FVector::DotProduct(Fwd, TestDir);
			const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

			if (Angle < BestAngle)
			{
				BestAngle = Angle;
				BestDir = TestDir;
				bFoundSafe = true;

				// 极小偏角（<1°）直接返回，不再搜索
				if (Angle < 1.f)
				{
					return BestDir;
				}
			}
		}
	}

	if (bFoundSafe)
	{
		// 推力平滑：与当前 Forward 按 SmoothFactor 加权混合
		const float S = FMath::Clamp(SmoothFactor, 0.f, 1.f);
		return (Fwd * (1.f - S) + BestDir * S).GetSafeNormal();
	}

	// ---- 5. 所有采样方向全部碰撞阻挡 → 180° 直接转身 ----
	{
		const FVector BackPos = Pos - Fwd * ProbeDistance;
		FCollisionShape BackShape = FCollisionShape::MakeSphere(AgentRadius);
		if (!World->OverlapAnyTestByChannel(BackPos, FQuat::Identity, Channel, BackShape, OverlapParams))
		{
			return -Fwd;
		}

		// 后退也被堵 → 沿局部坐标系侧向推开
		return Right.GetSafeNormal();
	}
}
