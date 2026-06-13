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

		for (int32 i = 0; i < Count; i++)
		{
			FFishMoveFragment& Fish = Fishes[i];
			FFishAlignFragment& Align = Aligns[i];
			FTransform& XForm = Transforms[i].GetMutableTransform();

			const FVector Pos = XForm.GetLocation();
			const FVector Forward = Fish.ForwardDir.GetSafeNormal();

			float SwimSpeed = FMath::RandRange(150.f,Fish.SwimSpeed);
			FVector SwimDir = Forward;

			const FVector AvoidDir = UBoidsFunction::ComputeObstacleAvoidance(
				Pos, Forward, Fish.EntityID, Align.AvoidRadius, Align.AvoidCollisionChannel, World);
			if (AvoidDir != Forward)
			{
				Fish.ForwardDir = AvoidDir;
				SwimDir = AvoidDir;
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
