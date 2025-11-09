// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "AdvancedMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class KNIGHTSORDER_API UAdvancedMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UAdvancedMovementProcessor();

protected:
	void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	

private:
	UPROPERTY()
	FMassEntityQuery EntityQuery;
};
