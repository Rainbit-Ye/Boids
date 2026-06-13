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
};
