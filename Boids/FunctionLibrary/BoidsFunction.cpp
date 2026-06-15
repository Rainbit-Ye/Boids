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

// ==================== 内部 Helper（接收已查询的邻居列表，避免重复 KNN） ====================

static FVector AlignFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Forward,
	float RadiusSq, int32 MaxNeighbors, float Weight)
{
	FVector AvgDir = FVector::ZeroVector;
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (N.DistanceSq > RadiusSq) break;      // 距离升序，超范围则停止
		if (Count >= MaxNeighbors) break;
		AvgDir += N.ForwardDir.GetSafeNormal();
		Count++;
	}
	if (Count == 0) return Forward;
	AvgDir *= 1.f / Count;
	AvgDir.Normalize();
	return (Forward * (1.f - Weight) + AvgDir * Weight).GetSafeNormal();
}

static FVector CohesionFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Pos,
	const FVector& Forward, float RadiusSq, int32 MaxNeighbors, float Weight, float MaxTurnAngle)
{
	FVector CenterOfMass = FVector::ZeroVector;
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (N.DistanceSq > RadiusSq) break;
		if (Count >= MaxNeighbors) break;
		CenterOfMass += N.Position;
		Count++;
	}
	if (Count == 0) return Forward;
	CenterOfMass /= static_cast<float>(Count);

	const FVector ToCenter = CenterOfMass - Pos;
	const float DistToCenter = ToCenter.Size();
	if (DistToCenter < 1.f) return Forward;

	const FVector DirToCenter = ToCenter / DistToCenter;
	const float DotFC = FVector::DotProduct(Forward, DirToCenter);
	const float AngleRad = FMath::Acos(FMath::Clamp(DotFC, -1.f, 1.f));
	const float MaxTurnAngleRad = FMath::DegreesToRadians(MaxTurnAngle);
	const float TurnAngleRad = FMath::Min(AngleRad * Weight, MaxTurnAngleRad);
	if (TurnAngleRad < 1.745e-3f) return Forward;

	const FVector Cross = FVector::CrossProduct(Forward, DirToCenter);
	const FVector Axis = Cross.IsNearlyZero()
		? (FMath::Abs(Forward.Z) < 0.99f
			? FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal()
			: FVector::CrossProduct(Forward, FVector::RightVector).GetSafeNormal())
		: Cross.GetSafeNormal();
	return FQuat(Axis, TurnAngleRad).RotateVector(Forward);
}

static FVector SeparationFromNeighbors(const TArray<FKNNResult>& Neighbors, const FVector& Pos,
	const FVector& Forward, float RadiusSq, int32 MaxNeighbors, float Strength)
{
	FVector Repulsion = FVector::ZeroVector;
	const float SepRadius = FMath::Sqrt(RadiusSq);
	int32 Count = 0;
	for (const FKNNResult& N : Neighbors)
	{
		if (N.DistanceSq > RadiusSq) break;
		if (Count >= MaxNeighbors) break;
		Count++;

		const FVector ToNeighbor = N.Position - Pos;
		const float Dist = FMath::Sqrt(N.DistanceSq);
		if (Dist < 1.f)
		{
			Repulsion += Forward * -1.f;
			continue;
		}
		const float T = FMath::Clamp(Dist / SepRadius, 0.f, 1.f);
		Repulsion -= ToNeighbor.GetSafeNormal() * (1.f - T);
	}
	if (Repulsion.IsNearlyZero()) return Forward;
	Repulsion.Normalize();
	return (Forward * (1.f - Strength) + Repulsion * Strength).GetSafeNormal();
}

// ==================== 合并接口：一次 KNN 完成对齐+凝聚+分离 ====================

void UBoidsFunction::ComputeAllBoidsForces(
	const UObject* WorldContextObject,
	const FVector& Pos, const FVector& Forward, const FGuid& EntityID,
	float AlignRadius, int32 AlignMaxNeighbors, float AlignWeight,
	float CohesionRadius, int32 CohesionMaxNeighbors, float CohesionWeight, float CohesionMaxTurnAngle,
	float SepRadius, int32 SepMaxNeighbors, float SepStrength,
	FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation)
{
	TArray<FKNNResult> Neighbors;
	ComputeAllBoidsForces(WorldContextObject,
		Pos, Forward, EntityID,
		AlignRadius, AlignMaxNeighbors, AlignWeight,
		CohesionRadius, CohesionMaxNeighbors, CohesionWeight, CohesionMaxTurnAngle,
		SepRadius, SepMaxNeighbors, SepStrength,
		OutAlign, OutCohesion, OutSeparation, Neighbors);
}

void UBoidsFunction::ComputeAllBoidsForces(
	const UObject* WorldContextObject,
	const FVector& Pos, const FVector& Forward, const FGuid& EntityID,
	float AlignRadius, int32 AlignMaxNeighbors, float AlignWeight,
	float CohesionRadius, int32 CohesionMaxNeighbors, float CohesionWeight, float CohesionMaxTurnAngle,
	float SepRadius, int32 SepMaxNeighbors, float SepStrength,
	FVector& OutAlign, FVector& OutCohesion, FVector& OutSeparation,
	TArray<FKNNResult>& ScratchNeighbors)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;

	OutAlign     = Forward;
	OutCohesion  = Forward;
	OutSeparation = Forward;

	if (!Grid || Grid->GetEntityCount() <= 1) return;

	// 取三组参数的最大值，一次查够
	const float MaxRadius = FMath::Max3(AlignRadius, CohesionRadius, SepRadius);
	const int32 MaxNeighbors = FMath::Max3(AlignMaxNeighbors, CohesionMaxNeighbors, SepMaxNeighbors);

	Grid->QueryKNN(Pos, MaxRadius, MaxNeighbors, EntityID, ScratchNeighbors);
	if (ScratchNeighbors.Num() == 0) return;

	OutAlign     = AlignFromNeighbors(ScratchNeighbors, Forward, AlignRadius * AlignRadius, AlignMaxNeighbors, AlignWeight);
	OutCohesion  = CohesionFromNeighbors(ScratchNeighbors, Pos, Forward, CohesionRadius * CohesionRadius, CohesionMaxNeighbors, CohesionWeight, CohesionMaxTurnAngle);
	OutSeparation = SeparationFromNeighbors(ScratchNeighbors, Pos, Forward, SepRadius * SepRadius, SepMaxNeighbors, SepStrength);
}

// ==================== 原始单功能接口（保留蓝图兼容性，内部委托给合并接口） ====================

FVector UBoidsFunction::ComputeAlignment(const UObject* WorldContextObject, float Weight, const FGuid& EntityID, const FVector& Forward, int32 MaxNeighbors, const 
									FVector& Pos, float AlignRadius)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;
	if (!Grid || Grid->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	Grid->QueryKNN(Pos, AlignRadius, MaxNeighbors, EntityID, Neighbors);

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
	AvgDir *= 1.f / Neighbors.Num();
	AvgDir.Normalize();

	// 自身方向 + 邻居平均方向加权混合
	return (Forward * (1.0f - Weight) + AvgDir * Weight).GetSafeNormal();
}

FVector UBoidsFunction::ComputeCohesion(const UObject* WorldContextObject,const FVector& Forward,const FGuid& EntityID,float CohesionRadius,int32 MaxNeighbors,float Weight,float MaxTurnAngle,const FVector& Pos)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;
	if (!Grid || Grid->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	Grid->QueryKNN(Pos, CohesionRadius, MaxNeighbors, EntityID, Neighbors);

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

	// 当前朝向到中心的夹角（全程弧度，避免 Deg/Rad 来回转换）
	const float DotFC = FVector::DotProduct(Forward, DirToCenter);
	const float AngleRad = FMath::Acos(FMath::Clamp(DotFC, -1.0f, 1.0f));
	const float MaxTurnAngleRad = FMath::DegreesToRadians(MaxTurnAngle);
	const float TurnAngleRad = FMath::Min(AngleRad * Weight, MaxTurnAngleRad);

	if (TurnAngleRad < 1.745e-3f)  // ≈ 0.1°
	{
		return Forward;
	}

	// 绕垂直于 Forward × DirToCenter 的轴旋转，趋近中心方向
	const FVector Cross = FVector::CrossProduct(Forward, DirToCenter);
	const FVector Axis = Cross.IsNearlyZero()
		// 方向平行/反向时 Cross 退化，任取一条垂直于 Forward 的轴
		? (FMath::Abs(Forward.Z) < 0.99f
			? FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal()
			: FVector::CrossProduct(Forward, FVector::RightVector).GetSafeNormal())
		: Cross.GetSafeNormal();

	return FQuat(Axis, TurnAngleRad).RotateVector(Forward);
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
	const UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;
	if (!Grid || Grid->GetEntityCount() <= 1)
	{
		return Forward;
	}

	TArray<FKNNResult> Neighbors;
	Grid->QueryKNN(Pos, SepRadius, MaxNeighbors, EntityID, Neighbors);

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

// ==================== 锥形视野避障（新版） ====================

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
	const UObject* WorldContextObject)
{
	TArray<FVector> SampleDirs;
	return ComputeObstacleAvoidanceCone(Pos, Forward, EntityID,
		ProbeDistance, AgentRadius, SampleAngles,
		AzimuthSamples, MaxSteps, SmoothFactor, Channel,
		WorldContextObject, SampleDirs);
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

	// ---- 1. 快速检测正前方是否畅通 ----
	{
		FHitResult QuickHit;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(AgentRadius);
		FCollisionQueryParams Params;
		Params.bTraceComplex = false;
		if (!World->SweepSingleByChannel(QuickHit, Pos, Pos + Fwd * ProbeDistance, FQuat::Identity, Channel, Sphere, Params))
		{
			return Forward; // 正前方畅通，无需避障
		}
		// 正前方被堵 → HitPos / HitNormal 供后续兜底用
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

	// ---- 4. 逐方向球体 Sweep，找最小偏角安全方向 ----
	FVector BestDir = Fwd;
	float BestAngle = 180.f;
	bool bFoundSafe = false;

	FCollisionQueryParams SweepParams;
	SweepParams.bTraceComplex = false;

	const int32 Steps = FMath::Min(MaxSteps, ScratchSampleDirs.Num());
	for (int32 i = 0; i < Steps; ++i)
	{
		const FVector TestDir = ScratchSampleDirs[i];

		FHitResult SweepHit;
		const FVector TraceEnd = Pos + TestDir * ProbeDistance;
		FCollisionShape SweepShape = FCollisionShape::MakeSphere(AgentRadius);

		const bool bBlocked = World->SweepSingleByChannel(
			SweepHit, Pos, TraceEnd, FQuat::Identity, Channel, SweepShape, SweepParams);

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
		else
		{
			// 记录碰撞法线（供兜底用）
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
		FHitResult BackHit;
		const FVector BackEnd = Pos - Fwd * ProbeDistance;
		FCollisionShape BackShape = FCollisionShape::MakeSphere(AgentRadius);
		if (!World->SweepSingleByChannel(BackHit, Pos, BackEnd, FQuat::Identity, Channel, BackShape, SweepParams))
		{
			return -Fwd;
		}

		// 后退也被堵 → 沿碰撞法线垂直推开
		FVector WallNormal = BackHit.ImpactNormal;
		if (WallNormal.IsNearlyZero())
		{
			WallNormal = Right; // 兜底：任意侧方向
		}
		return WallNormal.GetSafeNormal();
	}
}
