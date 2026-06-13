// Fill out your copyright notice in the Description page of Project Settings.
#include "FishAlignProcessor.h"
#include "FishGridProcessor.h"
#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "MyDemo/Boids/FunctionLibrary/BoidsFunction.h"

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
	BoidsQuery.AddRequirement<FFishCohesionFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddRequirement<FFishSeparationFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.RegisterWithProcessor(*this);
}

void UFishAlignProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	const float DeltaTime = Context.GetDeltaTimeSeconds();

	BoidsQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const int32 Count = ChunkCtx.GetNumEntities();
		TArrayView<FTransformFragment> Transforms = ChunkCtx.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetMutableFragmentView<FFishMoveFragment>();
		const TArrayView<FFishAlignFragment> Aligns = ChunkCtx.GetMutableFragmentView<FFishAlignFragment>();
		const TArrayView<FFishCohesionFragment> Cohesions = ChunkCtx.GetMutableFragmentView<FFishCohesionFragment>();
		const TArrayView<FFishSeparationFragment> Separations = ChunkCtx.GetMutableFragmentView<FFishSeparationFragment>();

		for (int32 i = 0; i < Count; i++)
		{
			FFishMoveFragment& Fish = Fishes[i];
			FFishAlignFragment& Align = Aligns[i];
			FFishCohesionFragment& Cohesion = Cohesions[i];
			FFishSeparationFragment& Separation = Separations[i];
			FTransform& XForm = Transforms[i].GetMutableTransform();

			const FVector Pos = XForm.GetLocation();
			const FVector Forward = Fish.ForwardDir.GetSafeNormal();
			const float SwimSpeed = Fish.SwimSpeed;

			// === Boids 群组行为：对齐 + 凝聚 + 分离，一次性加权混合 ===
			const FVector AlignTarget = UBoidsFunction::ComputeAlignment(
				Pos, Forward, Fish.EntityID,
				Align.Radius, Align.MaxNeighbors, Align.Weight, World);

			const FVector CohesionTarget = UBoidsFunction::ComputeCohesion(
				Pos, Forward, Fish.EntityID,
				Cohesion.Radius, Cohesion.MaxNeighbors, Cohesion.Weight, Cohesion.MaxTurnAngle, World);

			const FVector SepTarget = UBoidsFunction::ComputeSeparation(
				Pos, Forward, Fish.EntityID,
				Separation.Radius, Separation.MaxNeighbors, Separation.Strength, World);

			// 权重：自身惯性 35% + 对齐共识 25% + 凝聚中心 25% + 分离排斥 15%
			FVector BoidsDir = Forward * 0.35f
				+ AlignTarget    * 0.25f
				+ CohesionTarget * 0.25f
				+ SepTarget      * 0.15f;
			BoidsDir.Normalize();

			// === 个体游荡：每帧微调角度 ±10°，打散过于一致的方向 ===
			Fish.TimeSinceLastDirChange += DeltaTime;
			const float WanderPhase = static_cast<float>(Fish.EntityID.A ^ Fish.EntityID.D) / 65535.0f * UE_PI * 2.0f;
			const float WanderAngle = FMath::Sin(WanderPhase + Fish.TimeSinceLastDirChange * 0.5f) * 8.0f;
			Fish.TimeSinceLastDirChange = FMath::Fmod(Fish.TimeSinceLastDirChange, 3600.0f);  // 防溢出

			FVector WanderDir = BoidsDir.RotateAngleAxis(WanderAngle, FVector::UpVector);
			// 游荡仅影响 20%，保持群组行为主导
			WanderDir = (BoidsDir * 0.8f + WanderDir * 0.2f).GetSafeNormal();

			// Boids 行为平滑（防抖），避障不参与
			const float BoidsSmooth = FMath::Min(DeltaTime * 4.0f, 0.4f);
			FVector SwimDir = FMath::Lerp(Forward, WanderDir, BoidsSmooth).GetSafeNormal();

			// === 避障独立：传入 Boids 后的方向，有障碍即时覆盖 ===
			const FVector AvoidDir = UBoidsFunction::ComputeObstacleAvoidance(
				Pos, SwimDir, Fish.EntityID, Align.AvoidRadius, Align.AvoidCollisionChannel, World);
			if (FVector::DotProduct(SwimDir, AvoidDir) < 0.9999f)
			{
				SwimDir = AvoidDir;
			}

			Fish.ForwardDir = SwimDir;

			// 平滑旋转
			const FQuat CurrentRot = XForm.GetRotation();
			const FQuat TargetRot = SwimDir.ToOrientationQuat();
			const float RotSmoothFactor = FMath::Min(2.5f * DeltaTime, 1.f);
			const FQuat NewRot = FQuat::Slerp(CurrentRot, TargetRot, RotSmoothFactor);
			XForm.SetRotation(NewRot);
			XForm.SetLocation(Pos + SwimDir * SwimSpeed * DeltaTime);

		}
	});
}
