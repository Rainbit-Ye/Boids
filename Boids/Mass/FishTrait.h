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
	
public:
	UFishTrait();
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};