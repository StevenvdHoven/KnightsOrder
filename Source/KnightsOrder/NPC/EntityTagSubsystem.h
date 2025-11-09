// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityClasses.h"
#include "MassEntityHandle.h"
#include "MassEntitySubsystem.h"
#include "MassEntityQuery.h"
#include "NPC/EntityTagTrait.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "EntityTagSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FTaggedEntity
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName Tag;

	UPROPERTY()
	FMassEntityHandle Entity;
};

UCLASS()
class KNIGHTSORDER_API UEntityTagProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UEntityTagProcessor();

protected:
	void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};

/**
 * 
 */
UCLASS()
class KNIGHTSORDER_API UEntityTagSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UEntityTagSubsystem() {};

	bool TryFindEntityWithTag(FName Tag, FMassEntityHandle& outHandle) const;

	void ClearCachedEntities();

	void SetChachedEntities(const FName& tag,const TArray<FMassEntityHandle>& InTaggedEntities);

	TArray<FMassEntityHandle> GetCachedEntities(FName tag) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override {};
	virtual void Deinitialize() override {};

private:
	UPROPERTY()
	TArray<FTaggedEntity> TaggedEntities;
};
