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
	// 核心：必须添加 TransformFragment，否则实体没有位置/旋转
	BuildContext.AddFragment<FTransformFragment>();
	// 鱼游动自定义数据碎片
	BuildContext.AddFragment<FFishMoveFragment>();
	// 对齐碎片（邻居鱼平均速度）
	BuildContext.AddFragment<FFishAlignFragment>();
	// 鱼标记Tag，给MoveProcessor匹配查询
	BuildContext.AddTag<FFishTag>();
	UE_LOG(LogFish, Log, TEXT("FishTrait::BuildTemplate - Fragments registered"));
}