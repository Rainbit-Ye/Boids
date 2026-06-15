// Fill out your copyright notice in the Description page of Project Settings.


#include "FishInitProcessor.h"

#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "MyDemo/Boids/FishGridSubsystem.h"

UFishInitProcessor::UFishInitProcessor()
{
	ObservedType = FFishMoveFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UFishInitProcessor::ConfigureQueries()
{
	FishInitQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	FishInitQuery.AddRequirement<FFishMoveFragment>(EMassFragmentAccess::ReadWrite);
	FishInitQuery.AddRequirement<FFishAlignFragment>(EMassFragmentAccess::ReadWrite);
	FishInitQuery.AddRequirement<FFishEntityFragment>(EMassFragmentAccess::ReadWrite);
	FishInitQuery.AddTagRequirement<FFishTag>(EMassFragmentPresence::All);
	FishInitQuery.RegisterWithProcessor(*this);
}

void UFishInitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = GetWorld();
	if (!World) return;
	UFishGridSubsystem* Grid = World ? World->GetSubsystem<UFishGridSubsystem>() : nullptr;
	if (!Grid || !Grid->IsInitialized()) return;
	
	FishInitQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const int32 FishCount = ChunkCtx.GetNumEntities();
		TArrayView<FTransformFragment> Transforms = ChunkCtx.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetMutableFragmentView<FFishMoveFragment>();
		TArrayView<FFishAlignFragment> Aligns = ChunkCtx.GetMutableFragmentView<FFishAlignFragment>();
		TArrayView<FFishEntityFragment> EntityFrags = ChunkCtx.GetMutableFragmentView<FFishEntityFragment>();

		for (int i = 0; i < FishCount; i++)
		{
			float YawAngle = FMath::RandRange(0.f, 360.f);
			float PitchAngle = FMath::RandRange(-8.f, 8.f);
			FRotator InitRotation(PitchAngle, YawAngle, 0.f);
			FQuat InitQuat = InitRotation.Quaternion();

			FTransform& XForm = Transforms[i].GetMutableTransform();
			XForm.SetRotation(InitQuat);

			FFishMoveFragment& Fish = Fishes[i];
			Fish.TimeSinceLastDirChange = FMath::RandRange(3.f, 10.f);
			Fish.ForwardDir = InitQuat.GetRightVector();
			Fish.SwimSpeed = Fish.SwimSpeed * FMath::FRandRange(0.7f, 1.3f);
			Fish.EntityID = FGuid::NewGuid();

			// 实体碎片：GridID 初始为 -1，等待 GridProcessor 填写
			FFishEntityFragment& EntityFrag = EntityFrags[i];
			EntityFrag.EntityID = Fish.EntityID;
			EntityFrag.GridID = -1;

			// 对齐碎片：计算结果清零
			FFishAlignFragment& AlignFrag = Aligns[i];
			AlignFrag.AlignmentForce = FVector::ZeroVector;
			AlignFrag.NeighborCount = 0;
		}
	});
}
