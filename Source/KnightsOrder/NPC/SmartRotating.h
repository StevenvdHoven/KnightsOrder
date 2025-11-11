// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTraitBase.h"
#include "SmartRotating.generated.h"

USTRUCT()
struct FSmartTrackEntityTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FSmartRotatedWithMovementTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT()
struct FSmartTrackingActorTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FSmartRotatingFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FVector TargetDirection;

	UPROPERTY()
	FVector LookAtOffset;

	UPROPERTY()
	FMassEntityHandle TargetEntity;
};

USTRUCT(BlueprintType)
struct FSmartRotatingActorTrackingFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* TargetActor;
};

USTRUCT(BlueprintType)
struct FSmartRotatingSharedDataFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float RotationSpeed;
};

UCLASS()
class USmartRotatingTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

public:
	UPROPERTY(EditAnywhere)
	bool bStarWithMovementTag;

	UPROPERTY(EditAnywhere)
	FSmartRotatingSharedDataFragment RotatingParameters;
};


/**
 * 
 */
UCLASS()
class KNIGHTSORDER_API USmartRotatingProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	USmartRotatingProcessor();

	void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	void ExecuteDefaultRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	void ExecuteMovementRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	void ExecuteTrackingRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	void ExecuteActorTrackingRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	
private:
	FMassEntityQuery DefaultRotationQuery;
	FMassEntityQuery MovementRotatioQuery;
	FMassEntityQuery TrackingRotationQuery;
	FMassEntityQuery ActorTrackingRotationQuery;
};
