// Fill out your copyright notice in the Description page of Project Settings.
#include "FishAlignProcessor.h"
#include "FishGridProcessor.h"
#include "FishFragment.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "MyDemo/Boids/FishGridSubsystem.h"
#include "MyDemo/Boids/FunctionLibrary/BoidsFunction.h"

UFishAlignProcessor::UFishAlignProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	
	ExecutionOrder.ExecuteAfter.Add(UFishGridProcessor::StaticClass()->GetFName());
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
}

void UFishAlignProcessor::ConfigureQueries()
{
	BoidsQuery.AddTagRequirement<FFishTag>(EMassFragmentPresence::All);
	BoidsQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddRequirement<FFishMoveFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddRequirement<FFishAlignFragment>(EMassFragmentAccess::ReadWrite);
	BoidsQuery.AddSharedRequirement<FFishBoidConfigSharedFragment>(EMassFragmentAccess::ReadOnly);
	BoidsQuery.RegisterWithProcessor(*this);
}

void UFishAlignProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = Context.GetWorld();
	const float DeltaTime = Context.GetDeltaTimeSeconds();
	UFishGridSubsystem* Grid = World->GetSubsystem<UFishGridSubsystem>();

	// === 获取玩家 Pawn，供距离冻结判断 ===
	APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
	const FVector PawnLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	
	TArray<FKNNResult> ScratchNeighbors;
	TArray<FVector> ScratchSampleDirs;
	static const TArray<float> SampleAngles = { 15.f, 30.f, 45.f, 60.f, 90.f};

	BoidsQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& ChunkCtx)
	{
		const FFishBoidConfigSharedFragment& CFG = ChunkCtx.GetSharedFragment<FFishBoidConfigSharedFragment>();

		const int32 Count = ChunkCtx.GetNumEntities();
		TArrayView<FTransformFragment> Transforms = ChunkCtx.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FFishMoveFragment> Fishes = ChunkCtx.GetMutableFragmentView<FFishMoveFragment>();
		const TArrayView<FFishAlignFragment> Aligns = ChunkCtx.GetMutableFragmentView<FFishAlignFragment>();

		for (int32 i = 0; i < Count; i++)
		{
			FFishMoveFragment& Fish = Fishes[i];
			
			const FVector Pos = Transforms[i].GetMutableTransform().GetLocation();
			const float DistSq = FVector::DistSquared(Pos, PawnLocation);
			const float FreezeDistSq = FMath::Square(CFG.FreezeDistance);

			if (DistSq > FreezeDistSq)
			{
				Fish.bIsFrozen = true;
			}
			else
			{
				Fish.bIsFrozen = false;
			}

			if (Fish.bIsFrozen)
			{
				continue;
			}

			FFishAlignFragment& Align = Aligns[i];
			FTransform& XForm = Transforms[i].GetMutableTransform();

			const FVector Forward = Fish.ForwardDir.GetSafeNormal();
			const float SwimSpeed = Fish.SwimSpeed;

			// === Boids 群组行为：一次 KNN 完成 对齐 + 凝聚 + 分离 ===
			FVector AlignTarget, CohesionTarget, SepTarget;
			UBoidsFunction::ComputeAllBoidsForces(
				World,
				Pos, Forward, Fish.EntityID,
				CFG.AlignMaxNeighbors,
				CFG.CohesionMaxNeighbors,
				CFG.SeparationMaxNeighbors,
				AlignTarget, CohesionTarget, SepTarget,
				ScratchNeighbors);

			FVector BoidsDir = Forward;
			if (Grid->IsGridBuilt())
			{
				BoidsDir = Forward * Grid->AvoidWeight
					+ AlignTarget    * Grid->AlignWeight
					+ CohesionTarget * Grid->CohesionWeight
					+ SepTarget      * Grid->SeparationWeight;
				BoidsDir.Normalize();
			}

			// === 周期性速度变化 ===
			Fish.TimeSinceLastSpeedChange += DeltaTime;
			if (Fish.TimeSinceLastSpeedChange >= CFG.SpeedChangeInterval)
			{
				Fish.TimeSinceLastSpeedChange = 0.f;
				const uint32 Seed = static_cast<uint32>(Fish.EntityID.A ^ Fish.EntityID.B ^ Fish.EntityID.C ^ Fish.EntityID.D);
				const float Hash = FMath::Frac(static_cast<float>(Seed) * 0.0001f);
				Fish.SwimSpeed = FMath::Lerp(CFG.MinSwimSpeed, CFG.MaxSwimSpeed, Hash * static_cast<float>((Seed >> 16) & 0xFFFF) / 65535.0f);
				Fish.SwimSpeed = FMath::Clamp(Fish.SwimSpeed, CFG.MinSwimSpeed, CFG.MaxSwimSpeed);
			}

			// === 个体游荡 ===
			Fish.TimeSinceLastDirChange += DeltaTime;
			const float WanderPhase = static_cast<float>(Fish.EntityID.A ^ Fish.EntityID.D) / 65535.0f * UE_PI * 2.0f;
			const float WanderAngle = FMath::Sin(WanderPhase + Fish.TimeSinceLastDirChange * 0.5f) * 8.0f;
			Fish.TimeSinceLastDirChange = FMath::Fmod(Fish.TimeSinceLastDirChange, 3600.0f);

			FVector WanderDir = BoidsDir.RotateAngleAxis(WanderAngle, FVector::UpVector);
			WanderDir = (BoidsDir * 0.8f + WanderDir * 0.2f).GetSafeNormal();

			// Boids 行为平滑（防抖），避障不参与
			const float BoidsSmooth = FMath::Min(DeltaTime * 4.0f, 0.4f);
			FVector SwimDir = FMath::Lerp(Forward, WanderDir, BoidsSmooth).GetSafeNormal();

			// === 避障（锥形视野）===
			const FVector AvoidDir = UBoidsFunction::ComputeObstacleAvoidanceCone(
				Pos, SwimDir, Fish.EntityID,
				CFG.AvoidRadius,
				50.f,
				SampleAngles,
				8,
				32,
				0.5f,
				CFG.AvoidCollisionChannel,
				World,
				ScratchSampleDirs);
			if (FVector::DotProduct(SwimDir, AvoidDir) < 0.9999f)
			{
				SwimDir = AvoidDir;
			}

			Fish.ForwardDir = SwimDir;

			// 水平修正：抑制 Z 轴漂移
			const float AbsZ = FMath::Abs(SwimDir.Z);
			if (AbsZ > 0.05f)
			{
				SwimDir.Z *= 1.f - FMath::Min(AbsZ * 0.3f, 0.4f);
				SwimDir.Normalize();
			}

			// 平滑旋转
			const FQuat CurrentRot = XForm.GetRotation();
			const FQuat MeshFix = FQuat(FVector::UpVector, HALF_PI);
			const FQuat TargetRot = MeshFix * SwimDir.Rotation().Quaternion();
			const FQuat NewRot = FQuat::Slerp(CurrentRot, TargetRot, CFG.TurnLerpSpeed);
			XForm.SetRotation(NewRot);
			XForm.SetLocation(Pos + SwimDir * SwimSpeed * DeltaTime);
		}
	});
	
}
