#include "FishTrait.h"
#include "FishFragment.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"

DEFINE_LOG_CATEGORY_STATIC(LogFish, Log, All);

UFishTrait::UFishTrait()
{

}

void UFishTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 鱼游动自定义数据碎片，用 Trait 参数初始化
	FFishMoveFragment& MoveFragment = BuildContext.AddFragment_GetRef<FFishMoveFragment>();
	MoveFragment.SwimSpeed = FishSpeed;
	MoveFragment.TurnLerpSpeed = LerpSpeed;
	MoveFragment.SpeedChangeInterval = SpeedChangeInterval;
	MoveFragment.MinSwimSpeed = MinSpeed;
	MoveFragment.MaxSwimSpeed = MaxSpeed;
	MoveFragment.FreezeDistance = FreezeDistance;

	// 对齐碎片（邻居鱼平均速度）
	FFishAlignFragment& AlignFragment = BuildContext.AddFragment_GetRef<FFishAlignFragment>();
	AlignFragment.Radius = FishRadiusToAlign;
	AlignFragment.MaxNeighbors = NeighborCount;
	AlignFragment.Weight = AlignWeight;

	// 凝聚碎片（向邻居中心靠拢）
	FFishCohesionFragment& CohesionFragment = BuildContext.AddFragment_GetRef<FFishCohesionFragment>();
	CohesionFragment.Radius = FishRadiusToCohesion;
	CohesionFragment.MaxNeighbors = NeighborCohesionCount;
	CohesionFragment.Weight = CohesionWeight;

	// 分离碎片（排斥靠近的鱼）
	FFishSeparationFragment& SeparationFragment = BuildContext.AddFragment_GetRef<FFishSeparationFragment>();
	SeparationFragment.Radius = FishRadiusToSeparation;
	SeparationFragment.MaxNeighbors = NeighborSeparationCount;

	// 鱼实体碎片（记录网格归属）
	BuildContext.AddFragment<FFishEntityFragment>();

	// 鱼标记Tag，给MoveProcessor匹配查询
	BuildContext.AddTag<FFishTag>();
}