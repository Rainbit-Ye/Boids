// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineTypes.h"
#include "FishGridConfig.h"
#include "FishGridSubsystem.generated.h"

/**
 * K 近邻查询结果
 */
USTRUCT()
struct FKNNResult
{
	GENERATED_BODY()
	
	FGuid EntityID;
	FVector Position;
	FVector ForwardDir;
	float DistanceSq = 0.0f;
};

/** 网格单元信息：记录该 Cell 内有哪几条鱼 */
USTRUCT()
struct FGridCell
{
	GENERATED_BODY()

	/** 网格单元编号 = X + Y * DimX + Z * DimX * DimY */
	int32 GridID = -1;
	int32 GridX = 0;
	int32 GridY = 0;
	int32 GridZ = 0;
	/** 当前 Cell 内的鱼 EntityID 列表 */
	TArray<FGuid> FishIDs;
};

USTRUCT()
struct FFishEntity
{
	GENERATED_BODY()
	
	int32 GridID = -1;
	FGuid FishID;
	FVector Position;
	FVector ForwardDir = FVector::ForwardVector;
	FFishEntity(){}
	FFishEntity(int32 GridID,FGuid FishID,FVector Position,FVector InForwardDir = FVector::ForwardVector)
	{
		this->GridID = GridID;
		this->FishID = FishID;
		this->Position = Position;
		this->ForwardDir = InForwardDir;
	}
};

/**
 * 均匀网格空间加速结构子系统（替代 BVH）
 * 每帧重建，用于高效的邻居查询和避障检测
 */
UCLASS()
class MYDEMO_API UFishGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UFishGridConfig> Config;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void OnLoadDataAssetCompleted();

	/** 是否已初始化 */
	bool IsInitialized() const { return bInitialized; }

	/** 网格是否已通过构建完毕 */
	bool IsGridBuilt() const { return bGridBuilt; }

	// ---- 构建 ----

	/** 清空所有数据（含实体数组），仅在 Deinitialize 或完全重建时使用 */
	void Clear();

	/** 仅清空网格数据，保留实体位置数组 — 每帧调用 */
	void ClearGridData();

	/** 更新或新增实体位置（已存在则原地覆盖，不存在则追加），返回实体在数组中的索引 */
	int32 UpdateOrAddEntity(const FGuid& EntityID, const FVector& Position, const FVector& ForwardDir);

	/** 添加实体到构建缓冲区（仅首次加载使用） */
	int32 AddEntity(FGuid EntityID, const FVector& Position, const FVector& ForwardDir);

	/** 从 EntityCellMap 重建 Cells 数组（在批量 UpdateOrAddEntity 后调用） */
	void RebuildCellLists();

	/** 使用已配置的 GridConfig DataAsset 构建网格（仅需调用一次） */
	void BuildGrid();
	
	int32 GetEntityCellByEntityID(const FGuid& EntityID) const;

	/** 获取指定 Cell 的信息（FishIDs 列表），返回 nullptr 表示不存在 */
	const FGridCell* GetCellInfo(int32 CellIdx) const;
	
	/** K 近邻查询：返回半径内距离最近的 K 个邻居（含位置和朝向） */
	void QueryKNN(const FVector& Center, float Radius, int32 K, FGuid ExcludeID, TArray<FKNNResult>& OutResults) const;
	
	UFUNCTION(BlueprintCallable)
	int32 GetEntityCount() const { return EntityCellMap.Num(); }

	UFUNCTION(BlueprintCallable)
	int32 GetGridCellCount() const { return DimX * DimY * DimZ; }
	
	/** 获取网格维度 (X, Y, Z) */
	UFUNCTION(BlueprintCallable)
	FIntVector GetGridDims() const { return FIntVector(DimX, DimY, DimZ); }

protected:
	/** 将世界坐标转换为网格 Cell Index */
	int32 WorldToCell(const FVector& WorldPos) const;

	/** 遍历中心点所在 Cell 及其 27 个相邻格（3x3x3），对实体距离筛选后回调（直接传 Entity 指针，避免回调内二次查 Map） */
	void ForEntitiesInSphere(const FVector& Center, float RadiusSq, TFunctionRef<void(const FFishEntity& Entity, float DistSq)> Callback) const;

private:

	
	bool bInitialized = false;
	bool bGridBuilt = false;

	// ---- 均匀网格 ----
	FVector GridMin = FVector::ZeroVector;
	float GridCellSize = 600.f;
	int32 DimX = 1;
	int32 DimY = 1;
	int32 DimZ = 1;
	
	/** EntityID → Cell Index 快速映射 */
	TMap<FGuid, FFishEntity> EntityCellMap;
	/** 所有 Cell 的结构化信息（GridID + FishIDs），长度 = DimX*DimY*DimZ */
	TArray<FGridCell> Cells;
	/** 上帧有鱼的 Cell 索引集合，避免每帧 Reset 全量 Cells */
	TArray<int32> DirtyCellIndices;
};
