#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "AdvancedMovementTrait.generated.h"

USTRUCT(BlueprintType)
struct FAdvancedMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float MaximumSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float BrakingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float Deceleration;
};

/**
 * 
 */
UCLASS()
class KNIGHTSORDER_API UAdvancedMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float DefaultMaximumSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float DefaultAcceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float DefaultBrakingDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Movement")
	float DefaultDeceleration = 2048.0f;

};
