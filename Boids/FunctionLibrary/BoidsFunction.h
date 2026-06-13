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
	 * 计算避障后的新前进方向，BVH 从 WorldContextObject 的 World 中自动获取
	 * @param Pos				鱼当前位置
	 * @param Forward			鱼当前朝向（单位向量）
	 * @param EntityID			鱼实体 ID，用于排除自身和确定性相位偏移
	 * @param AvoidRadius		避障检测半径
	 * @param PhysChannel		世界物理碰撞通道
	 * @param WorldContextObject	世界上下文对象，用于获取 BVH 子系统
	 * @return 避开障碍后的前进方向，未检测到障碍 / BVH 无效时返回 Forward
	 */
	UFUNCTION(BlueprintCallable)
	static FVector ComputeObstacleAvoidance(
		const FVector& Pos,
		const FVector& Forward,
		const FGuid& EntityID,
		float AvoidRadius,
		ECollisionChannel PhysChannel,
		const UObject* WorldContextObject
	);

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
};
