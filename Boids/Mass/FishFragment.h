#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "FishFragment.generated.h"

USTRUCT(BlueprintType)
struct FFishMoveFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FGuid EntityID;
	float SwimSpeed = 300.f;
	// 转向lerp速度
	float TurnLerpSpeed = 0.5;
	// 距离上次切换已经过的时间
	float TimeSinceLastDirChange = 0.f;
	// 切换方向时间间隔
	float DirectionChangeInterval = 7.f;
	// 最大转向角度
	float MaxTurnAngle = 180;

	FVector ForwardDir = FVector::ZeroVector;
	

public:
};

USTRUCT()
struct FFishAlignFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	/** 对齐感知半径 */
	float Radius = 500.f;

	/** 最多取多少条邻居参与计算 */
	int32 MaxNeighbors = 5;

	/** 对齐权重 (0~1，控制对齐影响力的强度) */
	float Weight = 0.8f;

	/** 最大转向力大小 */
	float MaxSteeringForce = 300.f;

	/** 计算结果：对齐力矢量 */
	FVector AlignmentForce = FVector::ZeroVector;
	
	/** 参与计算的邻居数量 */
	int32 NeighborCount = 0;
	
	/** 检测球体半径 */
	float AvoidRadius = 80.f;

	/** 世界物理碰撞通道（用于检测静态环境障碍） */
	TEnumAsByte<ECollisionChannel> AvoidCollisionChannel = ECC_WorldStatic;

public:
	FFishAlignFragment() {}
};

USTRUCT(BlueprintType)
struct FFishTag : public FMassTag
{
	GENERATED_BODY()
};