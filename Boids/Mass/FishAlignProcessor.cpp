// Fill out your copyright notice in the Description page of Project Settings.
#include "FishAlignProcessor.h"
#include "FishGridProcessor.h"
#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "MyDemo/Boids/FishBVHSubsystem.h"

UFishAlignProcessor::UFishAlignProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;

	// BVH 在 FishGridProcessor 中已构建好，Align 排其后执行
	ExecutionOrder.ExecuteAfter.Add(UFishGridProcessor::StaticClass()->GetFName());
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UFishAlignProcessor::ConfigureQueries()
{
	BoidsQuery.AddTagRequirement<FFishTag>(EMassFragmentPresence::All);
	BoidsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddRequirement<FFishMoveFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddRequirement<FFishAlignFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.RegisterWithProcessor(*this);
}

void UFishAlignProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	if (!BVH || BVH->GetEntityCount() == 0) return;

	const float DeltaTime = Context.GetDeltaTimeSeconds();

	BoidsQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const int32 Count = ChunkCtx.GetNumEntities();
		TArrayView<FTransformFragment> Transforms = ChunkCtx.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetMutableFragmentView<FFishMoveFragment>();
		const TArrayView<FFishAlignFragment> Aligns = ChunkCtx.GetMutableFragmentView<FFishAlignFragment>();

		for (int32 i = 0; i < Count; i++)
		{
			FFishMoveFragment& Fish = Fishes[i];
			FFishAlignFragment& Align = Aligns[i];
			FTransform& XForm = Transforms[i].GetMutableTransform();

			const FVector Pos = XForm.GetLocation();
			const FVector Forward = Fish.ForwardDir.GetSafeNormal();

			float SwimSpeed = Fish.SwimSpeed;
			FVector SwimDir = Forward;

			const float AvoidRadius = Align.AvoidRadius;
			const ECollisionChannel PhysChannel = Align.AvoidCollisionChannel;

			// === Step 1: BVH + 物理 —— 检测前方是否有障碍 ===
			const FVector AheadPoint = Pos + Forward * AvoidRadius;
			FVector ObstaclePos;
			const bool bObstacleAhead = BVH->HasNeighborInRadius(AheadPoint, AvoidRadius * 0.8f, Fish.EntityID, PhysChannel, ObstaclePos);

			if (bObstacleAhead)
			{
				const FVector ToObstacle = (ObstaclePos - Pos).GetSafeNormal();

				// === Step 2: 从碰撞点向外螺旋搜索 XZ 平面，找第一个无障碍点 ===
				// 固定 Y，在 XZ 平面逐步扩大偏移，直到找到不碰撞的通行区域
				const float SearchStep = AvoidRadius * 0.4f;   // 每圈的步长
				const int32 MaxRings = 6;                       // 最大圈数

				FVector BestDir = Forward;
				bool bFound = false;

				for (int32 ring = 1; ring <= MaxRings && !bFound; ++ring)
				{
					const float Radius = SearchStep * ring;
					const int32 NumPoints = ring * 8;            // 越外层点越密

					for (int32 p = 0; p < NumPoints; ++p)
					{
						FVector TestPoint = ObstaclePos;
						TestPoint.X += FMath::FRandRange(-Radius, Radius);
						TestPoint.Z += FMath::FRandRange(-Radius, Radius);
						// Y 固定 = 碰撞点 Y，不上下浮动

						if (!BVH->HasNeighborInRadius(TestPoint, AvoidRadius * 0.5f, Fish.EntityID, PhysChannel))
						{
							// 找到空地 → 计算从自身到该点的绕行方向
							BestDir = (TestPoint - Pos).GetSafeNormal();
							bFound = true;
							break;
						}
					}
				}

				// 如果全被堵死，用背离障碍方向
				if (!bFound)
				{
					BestDir = -ToObstacle;
				}

			// === Step 3: 应用避障结果 ===
				Fish.ForwardDir = BestDir;
				SwimDir = BestDir;
			}

			// 平滑旋转
			const FQuat CurrentRot = XForm.GetRotation();
			const FQuat TargetRot = SwimDir.ToOrientationQuat();
			const float RotSmoothFactor = FMath::Min(2.5f * DeltaTime, 1.f);
			const FQuat NewRot = FQuat::Slerp(CurrentRot, TargetRot, RotSmoothFactor);
			XForm.SetRotation(NewRot);
			// 手动移动
			const FVector NewPos = Pos + SwimDir * SwimSpeed * DeltaTime;
			XForm.SetLocation(NewPos);

		}
	});
}
