#include "FishTrait.h"
#include "FishFragment.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntityUtils.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"

DEFINE_LOG_CATEGORY_STATIC(LogFish, Log, All);

UFishTrait::UFishTrait()
{

}

void UFishTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// === Shared Fragment：所有鱼共享的静态配置（一个 Chunk 只存一份）===
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	FFishBoidConfigSharedFragment ConfigData;
	ConfigData.InitialSwimSpeed       = FishSpeed;
	ConfigData.TurnLerpSpeed          = LerpSpeed;
	ConfigData.SpeedChangeInterval    = SpeedChangeInterval;
	ConfigData.MinSwimSpeed           = MinSpeed;
	ConfigData.MaxSwimSpeed           = MaxSpeed;
	ConfigData.FreezeDistance         = FreezeDistance;
	ConfigData.AlignMaxNeighbors      = NeighborCount;
	ConfigData.AlignWeight            = AlignWeight;
	ConfigData.CohesionMaxNeighbors   = NeighborCohesionCount;
	ConfigData.CohesionWeight         = CohesionWeight;
	ConfigData.SeparationMaxNeighbors = NeighborSeparationCount;

	const uint32 ConfigHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(ConfigData));
	FSharedStruct SharedConfig = EntityManager.GetOrCreateSharedFragmentByHash<FFishBoidConfigSharedFragment>(ConfigHash, ConfigData);
	BuildContext.AddSharedFragment(SharedConfig);

	// === Per-Entity Fragments：只保留运行时状态 ===
	FFishMoveFragment& MoveFragment = BuildContext.AddFragment_GetRef<FFishMoveFragment>();
	MoveFragment.SwimSpeed = FishSpeed; // 初始值，InitProcessor 随机化

	BuildContext.AddFragment<FFishAlignFragment>();      // AlignmentForce + NeighborCount
	BuildContext.AddFragment<FFishEntityFragment>();     // Grid归属

	// 鱼标记Tag，用于各Processor查询匹配
	BuildContext.AddTag<FFishTag>();
}