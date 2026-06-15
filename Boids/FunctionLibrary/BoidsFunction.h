// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BoidsFunction.generated.h"

UCLASS()
class MYDEMO_API UBoidsFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 鱼群对齐：获取半径内最近的 MaxNeighbors 条邻居，将自身方向与邻居平均方向混合
	 * @param Pos				鱼当前位置
	 * @param Forward			鱼当前朝向（单位向量）
	 * @param EntityID			鱼实体 ID，排除自身
	 * @param AlignRadius		对齐感知半径
	 * @param MaxNeighbors		最多取多少条邻居
	 * @param Weight			对齐权重 (0~1)
	 * @param WorldContextObject	世界上下文对象
	 * @return 对齐后的前进方向，没有邻居时返回 Forward
	 */
	UFUNCTION(BlueprintCallable)
	static FVector ComputeAlignment(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float AlignRadius,
		int32 MaxNeighbors,
		float Weight,
		const UObject* WorldContextObject
	);

	/**
	 * 凝聚：向邻居位置的中心靠拢，带最大转角约束
	 * @param Pos				鱼当前位置
	 * @param Forward			鱼当前朝向（单位向量）
	 * @param EntityID			鱼实体 ID，排除自身
	 * @param CohesionRadius	凝聚感知半径
	 * @param MaxNeighbors		最多取多少条邻居
	 * @param Weight			凝聚权重 (0~1)
	 * @param MaxTurnAngle		最大转角（度），单次转向中心的上限
	 * @param WorldContextObject	世界上下文对象
	 * @return 靠拢后的前进方向，没有邻居时返回 Forward
	 */
	UFUNCTION(BlueprintCallable)
	static FVector ComputeCohesion(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float CohesionRadius,
		int32 MaxNeighbors,
		float Weight,
		float MaxTurnAngle,
		const UObject* WorldContextObject
	);

	/**
	 * 分离：对靠太近的邻居产生排斥力，防止重叠
	 * @param Pos				鱼当前位置
	 * @param Forward			鱼当前朝向（单位向量）
	 * @param EntityID			鱼实体 ID，排除自身
	 * @param SepRadius			分离感知半径
	 * @param MaxNeighbors		最多取多少条邻居
	 * @param Strength			排斥力度 (0~1)
	 * @param WorldContextObject	世界上下文对象
	 * @return 排斥后的前进方向，没有过近邻居时返回 Forward
	 */
	UFUNCTION(BlueprintCallable)
	static FVector ComputeSeparation(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float SepRadius,
		int32 MaxNeighbors,
		float Strength,
		const UObject* WorldContextObject
	);

	/**
	 * 锥形视野避障：以 Forward 为中轴生成采样向量 → ID哈希打乱 → Sweep → 最小偏角安全方向
	 * @param Pos				实体当前位置
	 * @param Forward			实体当前朝向（单位向量）
	 * @param EntityID			实体 ID，用于哈希打乱采样顺序，避免集群鱼挤向同一侧
	 * @param ProbeDistance		探测距离（Sweep 终点 = Pos + 方向 × 探测距离）
	 * @param AgentRadius		球体 Sweep 半径
	 * @param SampleAngles		锥形采样半角数组（度），如 {15,30,45,60,90,135,180}
	 * @param AzimuthSamples	每环方位采样数
	 * @param MaxSteps			最大采样步数（防死循环）
	 * @param SmoothFactor		推力平滑系数 (0~1)
	 * @param Channel			碰撞通道
	 * @param WorldContextObject	世界上下文对象
	 * @return 避障后方向
	 */
	static FVector ComputeObstacleAvoidanceCone(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float ProbeDistance,
		float AgentRadius,
		const TArray<float>& SampleAngles,
		int32 AzimuthSamples,
		int32 MaxSteps,
		float SmoothFactor,
		ECollisionChannel Channel,
		const UObject* WorldContextObject
	);
};
