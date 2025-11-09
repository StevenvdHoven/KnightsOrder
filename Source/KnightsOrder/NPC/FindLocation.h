#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTrait.h"
#include "MassStateTreeTypes.h"
#include "MassNavigationFragments.h"
#include "StateTreeExecutionContext.h"
#include "MassEntitySubsystem.h"
#include "MassNavigationFragments.h"
#include "MassStateTreeTypes.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "Math/UnrealMathUtility.h"
#include "MassStateTreeExecutionContext.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "FindLocation.generated.h"


/**
 * 
 */
USTRUCT(meta = (DisplayName = "Find Location", StateTreeTaskType = "Task", Instanced))
struct KNIGHTSORDER_API FFindRandomLocation : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	FFindRandomLocation();

	UStruct* GetInstanceDataType() const override;

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (StateTreeInput))
	FVector2D SearchAreaMinMax = FVector2D(500.0f, 500.0f);

	UPROPERTY(EditAnywhere, Category = "Output", meta = (StateTreeOutput))
	FMassTargetLocation OutputLocation;

	UPROPERTY()
	FMassEntityQuery LocationQuery;
};

UENUM(BlueprintType)
enum class EFindEntityByTagMethod : uint8
{
	Random UMETA(DisplayName = "Random"),
	Closest UMETA(DisplayName = "Closest"),
	Farthest UMETA(DisplayName = "Farthest")
};

USTRUCT(meta = (DisplayName = "Find Entity Of Tag", StateTreeTaskType = "Task", Instanced))
struct KNIGHTSORDER_API FFindEntityOfTag : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	FFindEntityOfTag();

	UStruct* GetInstanceDataType() const override { return FFindEntityOfTag::StaticStruct(); };

	void GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const override;

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	FMassEntityHandle GetClosestEntity(const FMassEntityHandle& SourceEntity, const TArray<FMassEntityHandle>& CandidateEntities, FMassEntityManager& EntityManager) const;
	FMassEntityHandle GetFarthestEntity(const FMassEntityHandle& SourceEntity, const TArray<FMassEntityHandle>& CandidateEntities, FMassEntityManager& EntityManager) const;

	UPROPERTY(EditAnywhere)
	EFindEntityByTagMethod FindMethod = EFindEntityByTagMethod::Random;

	UPROPERTY(EditAnywhere)
	TArray<FName> TagsToFind;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (StateTreeOutput))
	FMassEntityHandle FoundEntity;
};



USTRUCT(meta = (DisplayName = "Set Location To Entity", StateTreeTaskType = "Task", Instanced))
struct KNIGHTSORDER_API FSetLocationToEntity : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	UStruct* GetInstanceDataType() const override { return FSetLocationToEntity::StaticStruct(); };

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (StateTreeInput))
	FMassEntityHandle EntityHandle;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (StateTreeOutput))
	FMassTargetLocation OutputLocation;
};