// Fill out your copyright notice in the Description page of Project Settings.

#include "FishGridProcessor.h"

#include "FishAlignProcessor.h"
#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "MyDemo/Boids/BoidsSubsystem.h"
#include "MyDemo/Boids/FishBVHSubsystem.h"

UFishGridProcessor::UFishGridProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// BVH 必须在 Align 之前构建好
	ExecutionOrder.ExecuteBefore.Add(UFishAlignProcessor::StaticClass()->GetFName());
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UFishGridProcessor::ConfigureQueries()
{
	BVHQuery.AddTagRequirement<FFishTag>(EMassFragmentPresence::All);
	BVHQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	BVHQuery.AddRequirement<FFishMoveFragment>(EMassFragmentAccess::ReadOnly);
	BVHQuery.RegisterWithProcessor(*this);
}

void UFishGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = GetWorld();
	UFishBVHSubsystem* BVH = World ? World->GetSubsystem<UFishBVHSubsystem>() : nullptr;
	UBoidsSubsystem* Boids = World ? World->GetSubsystem<UBoidsSubsystem>() : nullptr;
	if (!BVH || !BVH->IsInitialized()) return;

	// 清空 BVH，准备重建
	BVH->Clear();

	BVHQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const int32 Count = ChunkCtx.GetNumEntities();
		const TConstArrayView<FTransformFragment> Transforms = ChunkCtx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetFragmentView<FFishMoveFragment>();

		for (int32 i = 0; i < Count; ++i)
		{
			const FVector Pos = Transforms[i].GetTransform().GetLocation();
			BVH->AddEntity(Fishes[i].EntityID, Pos);
			Boids->ModifyBoids(Fishes[i].EntityID, ChunkCtx.GetEntity(i));
		}
	});

	// 构建 BVH 树
	BVH->BuildTree();
}
