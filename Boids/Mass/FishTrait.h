#pragma once
#include "MassEntityTraitBase.h"
#include "MassRepresentationTypes.h"
#include "MassRepresentationFragments.h"
#include "FishFragment.h"
#include "MassRepresentationSubsystem.h"

#include "FishTrait.generated.h"

UCLASS(BlueprintType)
class MYDEMO_API UFishTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	// ===== Move =====
	UPROPERTY(EditAnywhere,meta=(Category = "Boids", DisplayName = "鱼的移动速度"))
	float FishSpeed = 300.f;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids", DisplayName = "转向插值速度（越大越平滑）"))
	float LerpSpeed = 0.5f;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids", DisplayName = "速度变化间隔（秒）"))
	float SpeedChangeInterval = 5.f;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids", DisplayName = "最低游动速度"))
	float MinSpeed = 150.f;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids", DisplayName = "最高游动速度"))
	float MaxSpeed = 500.f;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids|LOD", DisplayName = "冻结距离（距离玩家Pawn多远后停止更新，cm）"))
	float FreezeDistance = 5000.f;

	// ===== Align =====
	UPROPERTY(EditAnywhere,meta=(Category = "Boids|Align", DisplayName = "对齐规则自身权重"))
	float AlignWeight = 0.8;
	
	UPROPERTY(EditAnywhere,meta=(Category = "Boids|Align", DisplayName = "邻居参考数量"))
	int32 NeighborCount = 8;

	// ===== Cohesion =====
	UPROPERTY(EditAnywhere,meta=(Category = "Boids|Cohesion", DisplayName = "聚集邻居数量"))
	int32 NeighborCohesionCount = 8;

	UPROPERTY(EditAnywhere,meta=(Category = "Boids|Cohesion", DisplayName = "聚集规则权重"))
	float CohesionWeight = 0.3f;
	
	// ===== Separation =====
	UPROPERTY(EditAnywhere,meta=(Category = "Boids|Separation", DisplayName = "分离邻居数量"))
	int32 NeighborSeparationCount = 8;
	
public:
	UFishTrait();
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};