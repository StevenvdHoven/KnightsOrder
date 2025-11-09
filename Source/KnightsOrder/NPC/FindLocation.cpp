#include "NPC/FindLocation.h"
#include "MassExecutionContext.h"
#include "NPC/EntityTagTrait.h"
#include "MassEntityBuilder.h"
#include "MassExecutionContext.h"
#include "MassStateTreeDependency.h"
#include "NPC/EntityTagSubsystem.h"


FFindRandomLocation::FFindRandomLocation()
{
	OutputLocation.EndOfPathPosition = FVector::ZeroVector;
	OutputLocation.EndOfPathIntent = EMassMovementAction::Stand;
}

UStruct* FFindRandomLocation::GetInstanceDataType() const
{
	return FFindRandomLocation::StaticStruct();
}

EStateTreeRunStatus FFindRandomLocation::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// Access the instance data (per-task, per-entity)
	FFindRandomLocation& InstanceData = Context.GetInstanceData<FFindRandomLocation>(*this);

	// --- Get the Mass entity context ---
	const FMassStateTreeExecutionContext& MassContext = static_cast<const FMassStateTreeExecutionContext&>(Context);
	const FMassEntityHandle Entity = MassContext.GetEntity();
	FMassEntityManager& EntityManager = MassContext.GetEntityManager();

	// Access a fragment (for example, the current Transform)
	const FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
	if (!TransformFragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindLocation: Missing TransformFragment!"));
		return EStateTreeRunStatus::Failed;
	}

	const FVector AgentLocation = TransformFragment->GetTransform().GetLocation();

	UE_LOG(LogTemp, Log, TEXT("FindLocation: Entity %d current location: %s"),
		Entity.Index, *AgentLocation.ToString());

	// --- Compute a new location around the agent ---
	const FVector NewLocation = AgentLocation + FVector(
		FMath::RandRange(-InstanceData.SearchAreaMinMax.X, InstanceData.SearchAreaMinMax.X),
		FMath::RandRange(-InstanceData.SearchAreaMinMax.Y, InstanceData.SearchAreaMinMax.Y),
		0.0f
	);

	UE_LOG(LogTemp, Log, TEXT("FindLocation: Entity %d new target location: %s"),
		Entity.Index, *NewLocation.ToString());

	// --- Write to StateTree Output (this is what downstream tasks will read) ---
	InstanceData.OutputLocation.EndOfPathPosition = NewLocation;
	InstanceData.OutputLocation.EndOfPathIntent = EMassMovementAction::Move;

	return EStateTreeRunStatus::Succeeded;
}

FFindEntityOfTag::FFindEntityOfTag()
{

}

void FFindEntityOfTag::GetDependencies(UE::MassBehavior::FStateTreeDependencyBuilder& Builder) const
{
}

EStateTreeRunStatus FFindEntityOfTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassContext = static_cast<const FMassStateTreeExecutionContext&>(Context);
	FMassEntityManager& EntityManager = MassContext.GetEntityManager();
	FFindEntityOfTag& InstanceData = Context.GetInstanceData<FFindEntityOfTag>(*this);

	if (InstanceData.TagsToFind.Num() == 0)
	{
		return EStateTreeRunStatus::Failed;
	}

	UEntityTagSubsystem* TagSubsystem = MassContext.GetWorld()->GetSubsystem<UEntityTagSubsystem>();
	for (auto tagToFind : InstanceData.TagsToFind)
	{
		TArray<FMassEntityHandle> EntitiesWithTag{ TagSubsystem->GetCachedEntities(tagToFind) };
		switch (InstanceData.FindMethod)
		{
		case EFindEntityByTagMethod::Random:
			if (EntitiesWithTag.Num() > 0)
			{
				const int32 RandomIndex = FMath::RandRange(0, EntitiesWithTag.Num() - 1);
				InstanceData.FoundEntity = EntitiesWithTag[RandomIndex];
				UE_LOG(LogTemp, Log, TEXT("FindEntityOfTag: Found entity %d with tag %s"),
					InstanceData.FoundEntity.Index, *tagToFind.ToString());
				return EStateTreeRunStatus::Succeeded;
			}
			UE_LOG(LogTemp, Warning, TEXT("FindEntityOfTag: No entities found with tag %s"), *tagToFind.ToString());
			return EStateTreeRunStatus::Failed;

		case EFindEntityByTagMethod::Closest:
			InstanceData.FoundEntity = GetClosestEntity(MassContext.GetEntity(), EntitiesWithTag, EntityManager);
			UE_LOG(LogTemp, Log, TEXT("FindEntityOfTag: Found closest entity %d with tag %s"),
				InstanceData.FoundEntity.Index, *tagToFind.ToString());
			return EStateTreeRunStatus::Succeeded;
			break;

		case EFindEntityByTagMethod::Farthest:
			InstanceData.FoundEntity = GetFarthestEntity(MassContext.GetEntity(), EntitiesWithTag, EntityManager);
			UE_LOG(LogTemp, Log, TEXT("FindEntityOfTag: Found farthest entity %d with tag %s"),
				InstanceData.FoundEntity.Index, *tagToFind.ToString());
			return EStateTreeRunStatus::Succeeded;
		}

		
	}

	UE_LOG(LogTemp, Warning, TEXT("FindEntityOfTag: No entities found with specified tags"));
	return EStateTreeRunStatus::Failed;
}

FMassEntityHandle FFindEntityOfTag::GetClosestEntity(const FMassEntityHandle& SourceEntity, const TArray<FMassEntityHandle>& CandidateEntities, FMassEntityManager& EntityManager) const
{
	const FTransformFragment& sourceTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(SourceEntity);
	float closestDistanceSq = TNumericLimits<float>::Max();

	FMassEntityHandle closestEntity;
	for (const FMassEntityHandle& candidateEntity : CandidateEntities)
	{
		const FTransformFragment& candidateTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(candidateEntity);
		const float distanceSq = FVector::DistSquared(sourceTransform.GetTransform().GetLocation(), candidateTransform.GetTransform().GetLocation());
		if (distanceSq < closestDistanceSq)
		{
			closestDistanceSq = distanceSq;
			closestEntity = candidateEntity;
		}
	}
	return closestEntity;

}

FMassEntityHandle FFindEntityOfTag::GetFarthestEntity(const FMassEntityHandle& SourceEntity, const TArray<FMassEntityHandle>& CandidateEntities, FMassEntityManager& EntityManager) const
{
	const FTransformFragment& sourceTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(SourceEntity);
	float farthestDistanceSq = TNumericLimits<float>::Min();

	FMassEntityHandle farthestEntity;
	for (const FMassEntityHandle& candidateEntity : CandidateEntities)
	{
		const FTransformFragment& candidateTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(candidateEntity);
		const float distanceSq = FVector::DistSquared(sourceTransform.GetTransform().GetLocation(), candidateTransform.GetTransform().GetLocation());
		if (distanceSq > farthestDistanceSq)
		{
			farthestDistanceSq = distanceSq;
			farthestEntity = candidateEntity;
		}
	}
	return farthestEntity;
	
}

EStateTreeRunStatus FSetLocationToEntity::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// Access the instance data (per-task, per-entity)
	FSetLocationToEntity& InstanceData = Context.GetInstanceData<FSetLocationToEntity>(*this);

	const FMassStateTreeExecutionContext& MassContext = static_cast<const FMassStateTreeExecutionContext&>(Context);
	const FMassEntityHandle Entity = MassContext.GetEntity();
	FMassEntityManager& EntityManager = MassContext.GetEntityManager();

	if (!InstanceData.EntityHandle.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetLocationToEntity: TargetEntity is not valid!"));
		return EStateTreeRunStatus::Failed;
	}

	const FTransformFragment* TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(InstanceData.EntityHandle);
	if (!TransformFragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("FindLocation: Missing TransformFragment!"));
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.OutputLocation.EndOfPathPosition = TransformFragment->GetTransform().GetLocation();
	InstanceData.OutputLocation.EndOfPathIntent = EMassMovementAction::Move;

	return EStateTreeRunStatus::Succeeded;
}


