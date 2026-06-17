// Fill out your copyright notice in the Description page of Project Settings.

#include "FishGridProcessor.h"

#include "FishAlignProcessor.h"
#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "MyDemo/Boids/FishGridSubsystem.h"

UFishGridProcessor::UFishGridProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;

	// 网格必须在 Align 之前构建好
	ExecutionOrder.ExecuteBefore.Add(UFishAlignProcessor::StaticClass()->GetFName());
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UFishGridProcessor::ConfigureQueries()
{
	BVHQuery.AddTagRequirement<FFishTag>(EMassFragmentPresence::All);
	BVHQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	BVHQuery.AddRequirement<FFishMoveFragment>(EMassFragmentAccess::ReadOnly);
	BVHQuery.AddRequirement<FFishEntityFragment>(EMassFragmentAccess::ReadWrite);
	BVHQuery.RegisterWithProcessor(*this);
}

void UFishGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = GetWorld();
	UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;
	if (!Grid || !Grid->IsInitialized())
	{
		return;
	}

	// ---- 一次性：从 Subsystem 自身的 DataAsset 配置初始化网格 ----
	if (!Grid->IsGridBuilt())
	{
		Grid->BuildGrid();
	}

	const FIntVector Dims = Grid->GetGridDims();
	const int32 DimXY = Dims.X * Dims.Y;

	BVHQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const int32 Count = ChunkCtx.GetNumEntities();
		const TConstArrayView<FTransformFragment> Transforms = ChunkCtx.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetFragmentView<FFishMoveFragment>();
		const TArrayView<FFishEntityFragment> EntityFrags = ChunkCtx.GetMutableFragmentView<FFishEntityFragment>();

		for (int32 i = 0; i < Count; ++i)
		{
			const FVector Pos = Transforms[i].GetTransform().GetLocation();
			const FVector Dir = Fishes[i].ForwardDir;
			const int32 GridID = Grid->UpdateOrAddEntity(Fishes[i].EntityID, Pos, Dir);

			FFishEntityFragment& Frag = EntityFrags[i];
			Frag.EntityID = Fishes[i].EntityID;
			Frag.GridID = GridID;

			if (GridID >= 0)
			{
				Frag.ZGridIdx = GridID / DimXY;
				const int32 Rem = GridID % DimXY;
				Frag.YGridIdx = Rem / Dims.X;
				Frag.XGridIdx = Rem % Dims.X;
			}
			else
			{
				Frag.XGridIdx = Frag.YGridIdx = Frag.ZGridIdx = 0;
			}
		}
	});

	// 从 EntityCellMap 重建 Cells 数组（KNN 查询依赖 Cells）
	Grid->RebuildCellLists();
}
