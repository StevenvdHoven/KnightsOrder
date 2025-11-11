#include "NPC/AdvancedMovementProcessor.h"
#include "NPC/AdvancedMovementTrait.h"
#include "MassNavigationFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "MassSimulationClasses.h"
#include "MassCommonFragments.h"
#include "MassEntityView.h"
#include "MassEntityClasses.h"
#include "MassSubsystemBase.h"

UAdvancedMovementProcessor::UAdvancedMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	EntityQuery.RegisterWithProcessor(*this);
}

void UAdvancedMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FAdvancedMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UAdvancedMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float deltaTime = Context.GetDeltaTimeSeconds();
	EntityQuery.ForEachEntityChunk(Context, [deltaTime](FMassExecutionContext& QueryContext)
		{
			const TArrayView<FAdvancedMovementFragment> AdvancedMovementFragments = QueryContext.GetMutableFragmentView<FAdvancedMovementFragment>();
			const TArrayView<FTransformFragment> TransformFragments = QueryContext.GetMutableFragmentView<FTransformFragment>();

			const int32 NumEntities = QueryContext.GetNumEntities();
			for (int32 entityIndex = 0; entityIndex < NumEntities; ++entityIndex)
			{
				FAdvancedMovementFragment& AdvancedMovementFragment = AdvancedMovementFragments[entityIndex];
				if (!AdvancedMovementFragment.bIsMoving)
				{
					continue;
				}

				FTransformFragment& TransformFragment = TransformFragments[entityIndex];
				FTransform& Transform = TransformFragment.GetMutableTransform();

				// Implement advanced movement logic here
				FVector targetLocation = AdvancedMovementFragment.TargetLocation;
				targetLocation.Z = Transform.GetLocation().Z; // Keep current Z to avoid vertical movement
				FVector currentLocation = TransformFragment.GetTransform().GetLocation();
				FVector direction = (targetLocation - currentLocation).GetSafeNormal();
				float distance = FVector::Dist(targetLocation, currentLocation);

				if (distance > AdvancedMovementFragment.BrakingDistance)
				{
					// Accelerate
					AdvancedMovementFragment.Speed = FMath::Min(AdvancedMovementFragment.Speed + AdvancedMovementFragment.Acceleration * deltaTime, AdvancedMovementFragment.MaximumSpeed);

				}
				else
				{
					// Decelerate
					AdvancedMovementFragment.Speed = FMath::Max(AdvancedMovementFragment.Speed - AdvancedMovementFragment.Deceleration * deltaTime, 0.0f);
				}

				// Update location
				currentLocation += direction * AdvancedMovementFragment.Speed * deltaTime;
				Transform.SetLocation(currentLocation);
			}
		});
}
