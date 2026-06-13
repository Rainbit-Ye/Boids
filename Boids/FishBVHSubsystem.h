// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineTypes.h"
#include "FishBVHSubsystem.generated.h"

/**
 * BVH 树节点
 */
struct FBVHNode
{
	/** 节点包围盒 */
	FBox Bounds;

	/** 子节点索引，-1 表示叶子节点 */
	int32 LeftChild = -1;
	int32 RightChild = -1;

	/** 叶子节点：EntityIDs/Positions 数组中的起始索引 */
	int32 EntityStart = 0;

	/** 叶子节点：该节点包含的实体数量 */
	int32 EntityCount = 0;

	bool IsLeaf() const { return LeftChild == -1; }
};

/**
 * BVH 空间加速结构子系统
 * 每帧重建，用于高效的邻居查询和避障检测
 */
UCLASS()
class MYDEMO_API UFishBVHSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 是否已初始化 */
	bool IsInitialized() const { return bInitialized; }

	// ---- 构建 ----

	/** 清空所有数据，准备新一帧构建 */
	void Clear();

	/** 添加实体到构建缓冲区 */
	void AddEntity(FGuid EntityID, const FVector& Position);

	/** 从缓冲区构建 BVH 树 */
	void BuildTree();

	// ---- 查询 ----

	/** 球体范围查询，返回范围内所有 EntityID */
	TArray<FGuid> QuerySphere(const FVector& Center, float Radius) const;

	/** 球体范围查询 —— 非分配版本（写入已有数组） */
	void QuerySphere(const FVector& Center, float Radius, TArray<FGuid>& OutEntityIDs) const;

	/** 点查询：检测指定点附近半径内是否有其他实体（排除自身） */
	bool HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID) const;

	/** 同上，同时返回最近邻居的位置 */
	bool HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FVector& OutNeighborPos) const;

	/** 点查询 + 世界物理遮挡：实体或世界几何体中任意一个挡住了就算有邻居 */
	bool HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel) const;

	/** 同上，同时返回位置（实体或物理碰撞点） */
	bool HasNeighborInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel, FVector& OutBlockPos) const;

	/** 查找半径内最近的非自身实体，未找到返回 false */
	bool FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FGuid& OutNearest) const;

	/** 返回位置 */
	bool FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, FGuid& OutNearest, FVector& OutPosition) const;

	/** 同上 + 物理：实体和世界几何体中挑最近的 */
	bool FindNearestInRadius(const FVector& Point, float Radius, FGuid ExcludeID, ECollisionChannel PhysicsChannel, FGuid& OutNearest, FVector& OutPosition) const;

	// ---- 调试 ----
	UFUNCTION(BlueprintCallable)
	int32 GetNodeCount() const { return Nodes.Num(); }
	
	UFUNCTION(BlueprintCallable)
	int32 GetEntityCount() const { return EntityIDs.Num(); }

protected:
	/** 递归构建 BVH */
	int32 BuildRecursive(int32 Start, int32 End);

	/** 递归球体查询 */
	void QueryRecursive(int32 NodeIdx, const FVector& Center, float RadiusSq, TArray<FGuid>& Out) const;

	/** 世界物理重叠检测：指定点附近是否有碰撞 */
	bool IsBlockedByWorld(const FVector& Point, float Radius, ECollisionChannel Channel) const;

	/** 世界物理重叠检测，返回碰撞位置 */
	bool IsBlockedByWorld(const FVector& Point, float Radius, ECollisionChannel Channel, FVector& OutHitPos) const;

private:
	bool bInitialized = false;

	/** BVH 节点数组 */
	TArray<FBVHNode> Nodes;

	/** 实体 ID */
	TArray<FGuid> EntityIDs;

	/** 实体位置 */
	TArray<FVector> EntityPositions;
};
