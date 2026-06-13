// Fill out your copyright notice in the Description page of Project Settings.

#include "FishBVHSubsystem.h"
#include "Algo/Sort.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

DEFINE_LOG_CATEGORY_STATIC(LogBVH, Log, All);

void UFishBVHSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bInitialized = true;
	UE_LOG(LogBVH, Log, TEXT("BVH Subsystem initialized"));
}

void UFishBVHSubsystem::Deinitialize()
{
	Clear();
	Super::Deinitialize();
}

// ==================== 构建 ====================

void UFishBVHSubsystem::Clear()
{
	Nodes.Empty();
	EntityIDs.Empty();
	EntityPositions.Empty();
}

void UFishBVHSubsystem::AddEntity(FGuid EntityID, const FVector& Position)
{
	EntityIDs.Add(EntityID);
	EntityPositions.Add(Position);
}

void UFishBVHSubsystem::BuildTree()
{
	// 获取当前所有实体数量
	const int32 Count = EntityIDs.Num();

	Nodes.Empty();
	if (Count == 0)
	{
		return;
	}

	Nodes.Reserve(Count * 2);
	BuildRecursive(0, Count);
}

int32 UFishBVHSubsystem::BuildRecursive(int32 Start, int32 End)
{
	const int32 NodeIdx = Nodes.Num();
	FBVHNode& Node = Nodes.AddDefaulted_GetRef();

	const int32 RangeCount = End - Start;

	// 计算该范围的 AABB
	FBox Bounds(ForceInit);
	for (int32 i = Start; i < End; i++)
	{
		// 计算box的最左和最右
		Bounds += EntityPositions[i];
	}
	Node.Bounds = Bounds;

	if (RangeCount <= 1)
	{
		// 叶子节点
		Node.LeftChild = -1;
		Node.RightChild = -1;
		Node.EntityStart = Start;
		Node.EntityCount = RangeCount;
		return NodeIdx;
	}

	// 选最长轴分割
	FVector Extent = Bounds.GetSize();
	int32 SplitAxis = 0;
	if (Extent.Y > Extent.X) SplitAxis = 1;
	if (Extent.Z > Extent[SplitAxis]) SplitAxis = 2;

	// 构建索引数组并沿轴排序
	TArray<int32> Indices;
	Indices.Reserve(RangeCount);
	for (int32 i = Start; i < End; ++i)
	{
		Indices.Add(i);
	}

	switch (SplitAxis)
	{
		case 0: Algo::SortBy(Indices, [&](int32 Idx) { return EntityPositions[Idx].X; }); break;
		case 1: Algo::SortBy(Indices, [&](int32 Idx) { return EntityPositions[Idx].Y; }); break;
		case 2: Algo::SortBy(Indices, [&](int32 Idx) { return EntityPositions[Idx].Z; }); break;
		default: break;
	}

	// 按排序后的顺序重排 EntityIDs 和 EntityPositions
	TArray<FGuid> SortedIDs;
	TArray<FVector> SortedPos;
	SortedIDs.Reserve(RangeCount);
	SortedPos.Reserve(RangeCount);
	for (int32 Idx : Indices)
	{
		SortedIDs.Add(EntityIDs[Idx]);
		SortedPos.Add(EntityPositions[Idx]);
	}

	for (int32 i = 0; i < RangeCount; ++i)
	{
		EntityIDs[Start + i] = SortedIDs[i];
		EntityPositions[Start + i] = SortedPos[i];
	}

	// 中位分割
	const int32 Mid = Start + RangeCount / 2;

	// 占位后递归构建左右子树
	Node.LeftChild = BuildRecursive(Start, Mid);
	Node.RightChild = BuildRecursive(Mid, End);

	return NodeIdx;
}

// ==================== 查询 ====================

TArray<FGuid> UFishBVHSubsystem::QuerySphere(const FVector& Center, float Radius) const
{
	TArray<FGuid> Result;
	QuerySphere(Center, Radius, Result);
	return Result;
}

void UFishBVHSubsystem::QuerySphere(const FVector& Center, float Radius, TArray<FGuid>& OutEntityIDs) const
{
	OutEntityIDs.Reset();
	if (Nodes.Num() == 0)
	{
		return;
	}

	const float RadiusSq = Radius * Radius;
	QueryRecursive(0, Center, RadiusSq, OutEntityIDs);
}

bool UFishBVHSubsystem::HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID) const
{
	FVector Dummy;
	return HasNeighborInRadius(Point, Radius, ExcludeID, Dummy);
}

bool UFishBVHSubsystem::HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FVector& OutNeighborPos) const
{
	if (Nodes.Num() == 0)
	{
		return false;
	}

	const float RadiusSq = Radius * Radius;
	float BestDistSq = RadiusSq;
	int32 BestIdx = -1;

	TArray<int32> Stack;
	Stack.Add(0);

	while (Stack.Num() > 0)
	{
		const int32 NodeIdx = Stack.Pop();
		const FBVHNode& Node = Nodes[NodeIdx];

		// 剪枝：AABB 比当前最近还远就跳过
		if (Node.Bounds.ComputeSquaredDistanceToPoint(Point) > BestDistSq)
		{
			continue;
		}

		if (Node.IsLeaf())
		{
			for (int32 i = 0; i < Node.EntityCount; ++i)
			{
				const int32 Idx = Node.EntityStart + i;
				if (EntityIDs[Idx] == ExcludeID) continue;

				const float DistSq = FVector::DistSquared(EntityPositions[Idx], Point);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestIdx = Idx;
				}
			}
		}
		else
		{
			Stack.Add(Node.RightChild);
			Stack.Add(Node.LeftChild);
		}
	}

	if (BestIdx >= 0)
	{
		OutNeighborPos = EntityPositions[BestIdx];
		return true;
	}
	return false;
}

bool UFishBVHSubsystem::FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FGuid& OutNearest) const
{
	FVector Dummy;
	return FindNearestInRadius(Point, Radius, ExcludeID, OutNearest, Dummy);
}

bool UFishBVHSubsystem::FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FGuid& OutNearest, FVector& OutPosition) const
{
	if (Nodes.Num() == 0)
	{
		return false;
	}

	const float RadiusSq = Radius * Radius;
	float BestDistSq = RadiusSq;
	int32 BestIdx = -1;

	TArray<int32> Stack;
	Stack.Add(0);

	while (Stack.Num() > 0)
	{
		const int32 NodeIdx = Stack.Pop();
		const FBVHNode& Node = Nodes[NodeIdx];

		// 剪枝：AABB 到点的距离超过了当前最优距离
		if (Node.Bounds.ComputeSquaredDistanceToPoint(Point) > BestDistSq)
		{
			continue;
		}

		if (Node.IsLeaf())
		{
			for (int32 i = 0; i < Node.EntityCount; ++i)
			{
				const int32 Idx = Node.EntityStart + i;
				if (EntityIDs[Idx] == ExcludeID) continue;

				const float DistSq = FVector::DistSquared(EntityPositions[Idx], Point);
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestIdx = Idx;
				}
			}
		}
		else
		{
			// 优先遍历距离更近的子节点（提高剪枝效率）
			const FBVHNode& Left = Nodes[Node.LeftChild];
			const FBVHNode& Right = Nodes[Node.RightChild];
			const float LeftDist = Left.Bounds.ComputeSquaredDistanceToPoint(Point);
			const float RightDist = Right.Bounds.ComputeSquaredDistanceToPoint(Point);

			if (LeftDist < RightDist)
			{
				Stack.Add(Node.RightChild);
				Stack.Add(Node.LeftChild);
			}
			else
			{
				Stack.Add(Node.LeftChild);
				Stack.Add(Node.RightChild);
			}
		}
	}

	if (BestIdx >= 0)
	{
		OutNearest = EntityIDs[BestIdx];
		OutPosition = EntityPositions[BestIdx];
		return true;
	}
	return false;
}

// ==================== 查询 + 物理 ====================

bool UFishBVHSubsystem::IsBlockedByWorld(const FVector& Point, float Radius, ECollisionChannel Channel) const
{
	FVector Dummy;
	return IsBlockedByWorld(Point, Radius, Channel, Dummy);
}

bool UFishBVHSubsystem::IsBlockedByWorld(const FVector& Point, float Radius, ECollisionChannel Channel, FVector& OutHitPos) const
{
	UWorld* World = GetWorld();
	if (!World) return false;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;

	FHitResult Hit;
	if (World->SweepSingleByChannel(Hit, Point, Point, FQuat::Identity, Channel, Sphere, Params))
	{
		OutHitPos = Hit.ImpactPoint;
		return true;
	}
	return false;
}

bool UFishBVHSubsystem::HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel) const
{
	FVector Dummy;
	return HasNeighborInRadius(Point, Radius, ExcludeID, PhysicsChannel, Dummy);
}

bool UFishBVHSubsystem::HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel, FVector& OutBlockPos) const
{
	// 1. 先查 BVH 邻居
	if (HasNeighborInRadius(Point, Radius, ExcludeID, OutBlockPos))
	{
		return true;
	}

	// 2. 再查世界物理
	return IsBlockedByWorld(Point, Radius, PhysicsChannel, OutBlockPos);
}

bool UFishBVHSubsystem::FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel, FGuid& OutNearest, FVector& OutPosition) const
{
	// 先走 BVH 找最近实体
	const bool bFoundEntity = FindNearestInRadius(Point, Radius, ExcludeID, OutNearest, OutPosition);

	// 查物理
	FVector PhysHitPos;
	const bool bFoundPhys = IsBlockedByWorld(Point, Radius, PhysicsChannel, PhysHitPos);

	if (bFoundEntity && bFoundPhys)
	{
		// 取更近的那个
		const float EntityDistSq = FVector::DistSquared(OutPosition, Point);
		const float PhysDistSq = FVector::DistSquared(PhysHitPos, Point);
		if (PhysDistSq < EntityDistSq)
		{
			OutPosition = PhysHitPos;
			OutNearest = FGuid();  // 物理碰撞没有 EntityID，置空
		}
		return true;
	}

	if (bFoundPhys)
	{
		OutPosition = PhysHitPos;
		OutNearest = FGuid();
		return true;
	}

	return bFoundEntity;
}

void UFishBVHSubsystem::QueryRecursive(int32 NodeIdx, const FVector& Center, float RadiusSq, TArray<FGuid>& Out) const
{
	const FBVHNode& Node = Nodes[NodeIdx];

	if (Node.Bounds.ComputeSquaredDistanceToPoint(Center) > RadiusSq)
	{
		return;
	}

	if (Node.IsLeaf())
	{
		for (int32 i = 0; i < Node.EntityCount; ++i)
		{
			const int32 Idx = Node.EntityStart + i;
			if (FVector::DistSquared(EntityPositions[Idx], Center) <= RadiusSq)
			{
				Out.Add(EntityIDs[Idx]);
			}
		}
	}
	else
	{
		QueryRecursive(Node.LeftChild, Center, RadiusSq, Out);
		QueryRecursive(Node.RightChild, Center, RadiusSq, Out);
	}
}
