#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "EntityTagTrait.generated.h"

USTRUCT(BlueprintType)
struct FEntityTagFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FName Tag;
};


/**
 * 
 */
UCLASS()
class KNIGHTSORDER_API UEntityTagTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity Tag")
	FName DefaultTag;
	
};
