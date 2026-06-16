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
	UFishGridSubsystem* Grid = World->GetSubsystem<UFishGridSubsystem>();

	// === 获取玩家 Pawn，供距离冻结判断 ===
	APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
	const FVector PawnLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;

	// 循环外复用 TArray，避免每鱼每帧堆分配
	TArray<FKNNResult> ScratchNeighbors;
	TArray<FVector> ScratchSampleDirs;
	static const TArray<float> SampleAngles = { 15.f, 30.f, 45.f, 60.f, 90.f};

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

			// === 距离冻结检查：超出距离即停止游动 ===
			const FVector Pos = Transforms[i].GetMutableTransform().GetLocation();
			const float DistSq = FVector::DistSquared(Pos, PawnLocation);
			const float FreezeDistSq = FMath::Square(Fish.FreezeDistance);

			if (DistSq > FreezeDistSq)
			{
				Fish.bIsFrozen = true;
			}
			else
			{
				Fish.bIsFrozen = false;
			}

			// 冻结中：跳过所有行为计算，保持位置不变
			if (Fish.bIsFrozen)
			{
				continue;
			}

			FFishAlignFragment& Align = Aligns[i];
			FFishCohesionFragment& Cohesion = Cohesions[i];
			FFishSeparationFragment& Separation = Separations[i];
			FTransform& XForm = Transforms[i].GetMutableTransform();

			const FVector Forward = Fish.ForwardDir.GetSafeNormal();
			const float SwimSpeed = Fish.SwimSpeed;

			// === Boids 群组行为：一次 KNN 完成 对齐 + 凝聚 + 分离 ===
			FVector AlignTarget, CohesionTarget, SepTarget;
			UBoidsFunction::ComputeAllBoidsForces(
				World,
				Pos, Forward, Fish.EntityID,
				Align.Radius, Align.MaxNeighbors, Align.Weight,
				Cohesion.Radius, Cohesion.MaxNeighbors, Cohesion.Weight, Cohesion.MaxTurnAngle,
				Separation.Radius, Separation.MaxNeighbors, Separation.Strength,
				AlignTarget, CohesionTarget, SepTarget,
				ScratchNeighbors);

			FVector BoidsDir = Forward;
			if (Grid->Config)
			{
				BoidsDir = Forward * Grid->Config->AvoidWeight
					+ AlignTarget    * Grid->Config->AlignWeight
					+ CohesionTarget * Grid->Config->CohesionWeight
					+ SepTarget      * Grid->Config->SeparationWeight;
				BoidsDir.Normalize();
			}

			// === 周期性速度变化：间隔到了就随机切换游速 ===
			Fish.TimeSinceLastSpeedChange += DeltaTime;
			if (Fish.TimeSinceLastSpeedChange >= Fish.SpeedChangeInterval)
			{
				Fish.TimeSinceLastSpeedChange = 0.f;
				// 用 EntityID 哈希生成每鱼独立的随机种子
				const uint32 Seed = static_cast<uint32>(Fish.EntityID.A ^ Fish.EntityID.B ^ Fish.EntityID.C ^ Fish.EntityID.D);
				const float Hash = FMath::Frac(static_cast<float>(Seed) * 0.0001f);
				Fish.SwimSpeed = FMath::Lerp(Fish.MinSwimSpeed, Fish.MaxSwimSpeed, Hash * static_cast<float>((Seed >> 16) & 0xFFFF) / 65535.0f);
				Fish.SwimSpeed = FMath::Clamp(Fish.SwimSpeed, Fish.MinSwimSpeed, Fish.MaxSwimSpeed);
			}

			
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
			//FVector SwimDir = Forward;
			FVector SwimDir = FMath::Lerp(Forward, WanderDir, BoidsSmooth).GetSafeNormal();

			// === 避障（锥形视野）：有障碍即时覆盖 SwimDir ===
			const FVector AvoidDir = UBoidsFunction::ComputeObstacleAvoidanceCone(
				Pos, SwimDir, Fish.EntityID,
				Align.AvoidRadius,      // 探测距离
				50.f,                    // 实体半径（球体 Sweep 半径）
				SampleAngles,            // 锥形采样半角数组
				8,                       // 每环方位采样数
				32,                      // 最大采样步数（防死循环）
				0.5f,                    // 推力平滑系数
				Align.AvoidCollisionChannel,
				World,
				ScratchSampleDirs);
			if (FVector::DotProduct(SwimDir, AvoidDir) < 0.9999f)
			{
				SwimDir = AvoidDir;
			}

			Fish.ForwardDir = SwimDir;

			// 水平修正：抑制 Z 轴漂移，避免鱼持续向上/下游
			const float AbsZ = FMath::Abs(SwimDir.Z);
			if (AbsZ > 0.05f)
			{
				SwimDir.Z *= 1.f - FMath::Min(AbsZ * 0.3f, 0.4f);
				SwimDir.Normalize();
			}

			// 平滑旋转：+90°Yaw 抵消 Mass SkeletalMesh 内置的 FromEngineToSM(-90°)
			const FQuat CurrentRot = XForm.GetRotation();
			const FQuat MeshFix = FQuat(FVector::UpVector, HALF_PI);
			const FQuat TargetRot = MeshFix * SwimDir.Rotation().Quaternion();
			const FQuat NewRot = FQuat::Slerp(CurrentRot, TargetRot, Fish.TurnLerpSpeed);
			XForm.SetRotation(NewRot);
			XForm.SetLocation(Pos + SwimDir * SwimSpeed * DeltaTime);
			

		}
	});
}
