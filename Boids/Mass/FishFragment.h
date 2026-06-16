#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FishFragment.generated.h"

// ============================================================
// Shared Fragment — chunk 内所有鱼共享的静态配置参数
// ============================================================
USTRUCT()
struct FFishBoidConfigSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()

	// ----- Move 配置 -----
	float InitialSwimSpeed = 300.f;
	float TurnLerpSpeed = 0.5f;       // 转向插值速度
	float SpeedChangeInterval = 5.f;  // 速度变化间隔
	float MinSwimSpeed = 150.f;
	float MaxSwimSpeed = 500.f;
	float FreezeDistance = 5000.f;    // 冻结距离

	// ----- Align 配置 -----
	float AlignRadius = 500.f;
	int32 AlignMaxNeighbors = 5;
	float AlignWeight = 0.8f;
	float MaxSteeringForce = 300.f;
	float AvoidRadius = 100.f;
	TEnumAsByte<ECollisionChannel> AvoidCollisionChannel = ECC_WorldStatic;

	// ----- Cohesion 配置 -----
	float CohesionRadius = 600.f;
	int32 CohesionMaxNeighbors = 8;
	float CohesionWeight = 0.3f;
	float CohesionMaxTurnAngle = 180.f;

	// ----- Separation 配置 -----
	float SeparationRadius = 150.f;
	float SeparationStrength = 0.5f;
	int32 SeparationMaxNeighbors = 10;
};

// ============================================================
// Per-Entity Fragments — 每只鱼自己独有的运行时数据
// ============================================================

USTRUCT()
struct FFishEntityFragment : public FMassFragment
{
	GENERATED_BODY()
	FGuid EntityID;
	int32 GridID;

	int32 XGridIdx = 0;
	int32 YGridIdx = 0;
	int32 ZGridIdx = 0;
};

USTRUCT(BlueprintType)
struct FFishMoveFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FGuid EntityID;
	float SwimSpeed = 300.f;                  // 运行时游速（每鱼随机）
	float TimeSinceLastDirChange = 0.f;       // 距离上次转向已过的秒数
	FVector ForwardDir = FVector::ZeroVector; // 当前朝向
	float TimeSinceLastSpeedChange = 0.f;     // 距离上次变速已过的秒数
	bool bIsFrozen = false;                   // 是否冻结
};

USTRUCT()
struct FFishAlignFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FVector AlignmentForce = FVector::ZeroVector; // 计算结果：对齐力
	int32 NeighborCount = 0;                       // 参与邻居数

public:
	FFishAlignFragment() {}
};

USTRUCT(BlueprintType)
struct FFishTag : public FMassTag
{
	GENERATED_BODY()
};
