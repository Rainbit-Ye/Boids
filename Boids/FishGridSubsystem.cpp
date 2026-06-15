// Fill out your copyright notice in the Description page of Project Settings.

#include "FishGridSubsystem.h"

#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogGrid, Log, All);
/** 3x3x3 邻居偏移量 — 保证 CellSize ≥ 搜索半径时球体查询不遗漏对角方向的实体 */
static const FIntVector GNeighbors[27] =
{
	{-1,-1,-1}, {-1,-1,0}, {-1,-1,1},
	{-1, 0,-1}, {-1, 0,0}, {-1, 0,1},
	{-1, 1,-1}, {-1, 1,0}, {-1, 1,1},
	{ 0,-1,-1}, { 0,-1,0}, { 0,-1,1},
	{ 0, 0,-1}, { 0, 0,0}, { 0, 0,1},
	{ 0, 1,-1}, { 0, 1,0}, { 0, 1,1},
	{ 1,-1,-1}, { 1,-1,0}, { 1,-1,1},
	{ 1, 0,-1}, { 1, 0,0}, { 1, 0,1},
	{ 1, 1,-1}, { 1, 1,0}, { 1, 1,1},
};


void UFishGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bInitialized = true;
	UE_LOG(LogGrid, Log, TEXT("Grid Subsystem initialized"));
	
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	const FSoftObjectPath Path(TEXT("/Game/DataTable/DA_GridConfig.DA_GridConfig"));
	StreamableManager.RequestAsyncLoad(Path, FStreamableDelegate::CreateUObject(this,&UFishGridSubsystem::OnLoadDataAssetCompleted));
	
	
}

void UFishGridSubsystem::Deinitialize()
{
	Clear();
	Super::Deinitialize();
}

void UFishGridSubsystem::OnLoadDataAssetCompleted()
{
	const FSoftObjectPath SandboxConfigPath = FSoftObjectPath(TEXT("/Game/DataTable/DA_GridConfig.DA_GridConfig"));
	Config = Cast<UFishGridConfig>(SandboxConfigPath.TryLoad());
	
	if (!Config)
	{
		return;
	}
}
// ==================== 构建 ====================


void UFishGridSubsystem::Clear()
{
	ClearGridData();
}

void UFishGridSubsystem::ClearGridData()
{
	EntityCellMap.Empty();
	Cells.Empty();
	DirtyCellIndices.Empty();
}

int32 UFishGridSubsystem::UpdateOrAddEntity(const FGuid& EntityID, const FVector& Position, const FVector& ForwardDir)
{
	if (FFishEntity* Found = EntityCellMap.Find(EntityID))
	{
		// 已存在 → 覆盖位置和前向，并重新计算 GridID（鱼移动后会跨 Cell）
		Found->Position = Position;
		Found->ForwardDir = ForwardDir;
		Found->GridID = WorldToCell(Position);
		return Found->GridID;
	}
	// 新实体 → 追加
	return AddEntity(EntityID, Position, ForwardDir);
}

int32 UFishGridSubsystem::AddEntity(FGuid EntityID, const FVector& Position, const FVector& ForwardDir)
{
	int32 GridID = WorldToCell(Position);
	FFishEntity Fish(GridID,EntityID,Position,ForwardDir);
	EntityCellMap.Add(EntityID,Fish);
	return GridID;
}

int32 UFishGridSubsystem::WorldToCell(const FVector& WorldPos) const
{
	const int32 X = FMath::Clamp(static_cast<int32>((WorldPos.X - GridMin.X) / GridCellSize), 0, DimX - 1);
	const int32 Y = FMath::Clamp(static_cast<int32>((WorldPos.Y - GridMin.Y) / GridCellSize), 0, DimY - 1);
	const int32 Z = FMath::Clamp(static_cast<int32>((WorldPos.Z - GridMin.Z) / GridCellSize), 0, DimZ - 1);
	return X + Y * DimX + Z * DimX * DimY;
}

void UFishGridSubsystem::BuildGrid()
{

	if (!Config)
	{
		return;
	}

	GridCellSize = Config->CellSize;

	// 扩展边距，确保查询时 Cell 外扩不会越界
	const float Margin = GridCellSize * 2.f;
	GridMin = Config->BoundMin - FVector(Margin);
	const FVector GridMax = Config->BoundMax + FVector(Margin);

	const FVector Extent = GridMax - GridMin;
	DimX = FMath::Max(1, FMath::CeilToInt32(Extent.X / GridCellSize));
	DimY = FMath::Max(1, FMath::CeilToInt32(Extent.Y / GridCellSize));
	DimZ = FMath::Max(1, FMath::CeilToInt32(Extent.Z / GridCellSize));
	bGridBuilt = true;

	UE_LOG(LogGrid, Log, TEXT("Grid built: Dim=(%d,%d,%d) CellSize=%.0f"), DimX, DimY, DimZ, GridCellSize);

	// 一次性初始化 Cells 元数据（GridID/GridX/Y/Z 后续不再变）
	const int32 TotalCells = DimX * DimY * DimZ;
	Cells.SetNum(TotalCells);
	for (int32 i = 0; i < TotalCells; ++i)
	{
		Cells[i].GridID = i;
		Cells[i].GridX = i % DimX;
		Cells[i].GridY = (i / DimX) % DimY;
		Cells[i].GridZ = i / (DimX * DimY);
		Cells[i].FishIDs.Reserve(8);
	}
}

void UFishGridSubsystem::RebuildCellLists()
{
	// 1. 只 Reset 上帧有鱼的 Cell（脏追踪）
	for (int32 CellIdx : DirtyCellIndices)
	{
		Cells[CellIdx].FishIDs.Reset();
	}
	DirtyCellIndices.Reset();

	// 2. 填入当前帧数据，首次写入该 Cell 时记录为脏
	const int32 TotalCells = Cells.Num();
	for (const auto& Pair : EntityCellMap)
	{
		const int32 CellIdx = Pair.Value.GridID;
		if (CellIdx >= 0 && CellIdx < TotalCells)
		{
			const bool bFirstFish = Cells[CellIdx].FishIDs.Num() == 0;
			Cells[CellIdx].FishIDs.Add(Pair.Value.FishID);
			if (bFirstFish)
			{
				DirtyCellIndices.Add(CellIdx);
			}
		}
	}
}


int32 UFishGridSubsystem::GetEntityCellByEntityID(const FGuid& EntityID) const
{
	const FFishEntity* Found = EntityCellMap.Find(EntityID);
	return Found ? Found->GridID : -1;
}

const FGridCell* UFishGridSubsystem::GetCellInfo(int32 CellIdx) const
{
	if (CellIdx < 0 || CellIdx >= Cells.Num()) return nullptr;
	return &Cells[CellIdx];
}


void UFishGridSubsystem::ForEntitiesInSphere(const FVector& Center, float RadiusSq,
	TFunctionRef<void(const FFishEntity& Entity, float DistSq)> Callback) const
{
	if (Cells.Num() == 0) return;

	const FGridCell& CenterCell = Cells[WorldToCell(Center)];

	for (const FIntVector& Offset : GNeighbors)
	{
		const int32 nx = CenterCell.GridX + Offset.X;
		const int32 ny = CenterCell.GridY + Offset.Y;
		const int32 nz = CenterCell.GridZ + Offset.Z;
		if (nx < 0 || nx >= DimX || ny < 0 || ny >= DimY || nz < 0 || nz >= DimZ) continue;

		for (const FGuid& FishID : Cells[nx + ny * DimX + nz * DimX * DimY].FishIDs)
		{
			const FFishEntity* Entity = EntityCellMap.Find(FishID);
			if (!Entity) continue;
			const float DistSq = FVector::DistSquared(Entity->Position, Center);
			if (DistSq <= RadiusSq)
				Callback(*Entity, DistSq);
		}
	}
}

// ---- KNN 查询 ----

void UFishGridSubsystem::QueryKNN(const FVector& Center, float Radius, int32 K, FGuid ExcludeID, TArray<FKNNResult>& OutResults) const
{
	OutResults.Reset();
	if (K <= 0) return;

	const float RadiusSq = Radius * Radius;

	ForEntitiesInSphere(Center, RadiusSq, [&](const FFishEntity& Entity, float DistSq)
	{
		if (Entity.FishID == ExcludeID) return;

		FKNNResult Result;
		Result.EntityID = Entity.FishID;
		Result.Position = Entity.Position;
		Result.ForwardDir = Entity.ForwardDir; 
		Result.DistanceSq = DistSq;
		OutResults.Add(Result);
	});

	// 距离升序
	OutResults.Sort([](const FKNNResult& A, const FKNNResult& B)
	{
		return A.DistanceSq < B.DistanceSq;
	});

	if (OutResults.Num() > K)
	{
		OutResults.SetNum(K);
	}
}

