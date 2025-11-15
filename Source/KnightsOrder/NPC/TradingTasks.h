#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MassStateTreeTypes.h"
#include "NPC/NPCTrading.h"
#include "MassNavigationFragments.h"
#include "TradingTasks.generated.h"

/**
 * 
 */
USTRUCT()
struct KNIGHTSORDER_API FSearchForTrade : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	UStruct* GetInstanceDataType() const override { return FSearchForTrade::StaticStruct(); };

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	UPROPERTY(EditAnywhere)
	ETradingProduct DesiredProduct;

	UPROPERTY(EditAnywhere)
	int32 MinimumQuantity;

	UPROPERTY(EditAnywhere)
	int32 MaximumQuantity;
};

USTRUCT(BlueprintType)
struct FWaitForTradeEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;
};

USTRUCT()
struct KNIGHTSORDER_API FFindPositionToTrade : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	UStruct* GetInstanceDataType() const override { return FFindPositionToTrade::StaticStruct(); };

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	UPROPERTY(EditAnywhere, Category = "Output", meta = (StateTreeOutput))
	FMassTargetLocation OutputLocation;
};

USTRUCT()
struct KNIGHTSORDER_API FWaitForTrade : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	UStruct* GetInstanceDataType() const override { return FWaitForTrade::StaticStruct(); };

	EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
