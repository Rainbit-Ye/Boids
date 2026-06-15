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

	/** 距离上次速度变化已经过的时间 */
	float TimeSinceLastSpeedChange = 0.f;

	/** 速度变化间隔（秒），周期性随机切换游速 */
	float SpeedChangeInterval = 5.f;

	/** 最低游动速度 */
	float MinSwimSpeed = 150.f;

	/** 最高游动速度 */
	float MaxSwimSpeed = 500.f;

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
	
	/** 避障检测距离（前方多远开始检测障碍物） */
	float AvoidRadius = 300.f;

	/** 世界物理碰撞通道（用于检测静态环境障碍） */
	TEnumAsByte<ECollisionChannel> AvoidCollisionChannel = ECC_WorldStatic;

public:
	FFishAlignFragment() {}
};

/** 凝聚：向邻居中心靠拢 */
USTRUCT()
struct FFishCohesionFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	/** 凝聚感知半径 */
	UPROPERTY(EditAnywhere)
	float Radius = 600.f;

	/** 参与计算的最多邻居数 */
	UPROPERTY(EditAnywhere)
	int32 MaxNeighbors = 8;

	/** 凝聚权重 (0~1) */
	UPROPERTY(EditAnywhere)
	float Weight = 0.3f;

	/** 最大转角（度），限制单次朝中心转向的角度上限 */
	UPROPERTY(EditAnywhere)
	float MaxTurnAngle = 30.f;
};

/** 分离：防止鱼重叠或靠太近 */
USTRUCT()
struct FFishSeparationFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	/** 分离感知半径，半径内才会产生排斥力 */
	UPROPERTY(EditAnywhere)
	float Radius = 150.f;

	/** 排斥力度 (0~1) */
	UPROPERTY(EditAnywhere)
	float Strength = 0.5f;

	/** 参与计算的最多邻居数 */
	UPROPERTY(EditAnywhere)
	int32 MaxNeighbors = 10;
};

USTRUCT(BlueprintType)
struct FFishTag : public FMassTag
{
	GENERATED_BODY()
};