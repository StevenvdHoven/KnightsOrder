#include "NPC/SmartRotating.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Kismet/KismetMathLibrary.h"

void USmartRotatingTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);
	FSmartRotatingFragment& smartRotatingFragment{ BuildContext.AddFragment_GetRef<FSmartRotatingFragment>() };
	BuildContext.AddFragment<FSmartRotatingActorTrackingFragment>();

	const FConstSharedStruct rotatingParameters{ EntityManager.GetOrCreateConstSharedFragment(RotatingParameters) };
	BuildContext.AddConstSharedFragment(rotatingParameters);

	if (bStarWithMovementTag)
	{
		BuildContext.AddTag<FSmartRotatedWithMovementTag>();
	}
}

USmartRotatingProcessor::USmartRotatingProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);

	DefaultRotationQuery.RegisterWithProcessor(*this);
	MovementRotatioQuery.RegisterWithProcessor(*this);
	TrackingRotationQuery.RegisterWithProcessor(*this);
	ActorTrackingRotationQuery.RegisterWithProcessor(*this);

}

void USmartRotatingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	DefaultRotationQuery.AddRequirement<FSmartRotatingFragment>(EMassFragmentAccess::ReadWrite);
	DefaultRotationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	DefaultRotationQuery.AddConstSharedRequirement<FSmartRotatingSharedDataFragment>(EMassFragmentPresence::All);
	DefaultRotationQuery.AddTagRequirement<FSmartRotatedWithMovementTag>(EMassFragmentPresence::None);
	DefaultRotationQuery.AddTagRequirement<FSmartTrackEntityTag>(EMassFragmentPresence::None);
	DefaultRotationQuery.AddTagRequirement<FSmartTrackingActorTag>(EMassFragmentPresence::None);

	MovementRotatioQuery.AddRequirement<FSmartRotatingFragment>(EMassFragmentAccess::ReadWrite);
	MovementRotatioQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	MovementRotatioQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	MovementRotatioQuery.AddConstSharedRequirement<FSmartRotatingSharedDataFragment>(EMassFragmentPresence::All);
	MovementRotatioQuery.AddTagRequirement<FSmartTrackEntityTag>(EMassFragmentPresence::None);
	MovementRotatioQuery.AddTagRequirement<FSmartTrackingActorTag>(EMassFragmentPresence::None);
	MovementRotatioQuery.AddTagRequirement<FSmartRotatedWithMovementTag>(EMassFragmentPresence::All);

	TrackingRotationQuery.AddRequirement<FSmartRotatingFragment>(EMassFragmentAccess::ReadWrite);
	TrackingRotationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	TrackingRotationQuery.AddConstSharedRequirement<FSmartRotatingSharedDataFragment>(EMassFragmentPresence::All);
	TrackingRotationQuery.AddTagRequirement<FSmartRotatedWithMovementTag>(EMassFragmentPresence::None);
	TrackingRotationQuery.AddTagRequirement<FSmartTrackingActorTag>(EMassFragmentPresence::None);
	TrackingRotationQuery.AddTagRequirement<FSmartTrackEntityTag>(EMassFragmentPresence::All);

	ActorTrackingRotationQuery.AddRequirement<FSmartRotatingFragment>(EMassFragmentAccess::ReadWrite);
	ActorTrackingRotationQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	ActorTrackingRotationQuery.AddRequirement<FSmartRotatingActorTrackingFragment>(EMassFragmentAccess::ReadOnly);
	ActorTrackingRotationQuery.AddConstSharedRequirement<FSmartRotatingSharedDataFragment>(EMassFragmentPresence::All);
	ActorTrackingRotationQuery.AddTagRequirement<FSmartRotatedWithMovementTag>(EMassFragmentPresence::None);
	ActorTrackingRotationQuery.AddTagRequirement<FSmartTrackEntityTag>(EMassFragmentPresence::None);
	ActorTrackingRotationQuery.AddTagRequirement<FSmartTrackingActorTag>(EMassFragmentPresence::All);
}

void USmartRotatingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ExecuteDefaultRotation(EntityManager, Context);
	ExecuteMovementRotation(EntityManager, Context);
	ExecuteTrackingRotation(EntityManager, Context);
	ExecuteActorTrackingRotation(EntityManager, Context);
}

void USmartRotatingProcessor::ExecuteDefaultRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	DefaultRotationQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& context)
		{
			const TArrayView<FSmartRotatingFragment> smartRotatingFragments{ context.GetMutableFragmentView<FSmartRotatingFragment>() };
			const TArrayView<FTransformFragment> transformFragments{ context.GetMutableFragmentView<FTransformFragment>() };
			const FSmartRotatingSharedDataFragment& sharedDataFragment{ context.GetConstSharedFragment<FSmartRotatingSharedDataFragment>() };

			const float rotationSpeed{ sharedDataFragment.RotationSpeed };
			const float deltaTime{ context.GetDeltaTimeSeconds() };

			const int32 NumEntities{ context.GetNumEntities() };
			UE_LOG(LogTemp, Warning, TEXT("Executing Default Rotation for %d entities."), NumEntities);
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				FSmartRotatingFragment& smartRotatingFragment{ smartRotatingFragments[entityIndex] };
				FTransformFragment& transformFragment{ transformFragments[entityIndex] };

				FVector currentForward{ transformFragment.GetTransform().GetUnitAxis(EAxis::Y) };
				FVector targetDirection{ smartRotatingFragment.TargetDirection.GetSafeNormal() };

				if (!targetDirection.IsNearlyZero())
				{
					FVector newForward{ FMath::VInterpConstantTo(currentForward, targetDirection, deltaTime, rotationSpeed) };
					newForward.Normalize();

					const FRotator newRotation{ FRotationMatrix::MakeFromX(newForward).Rotator() };
					transformFragment.GetMutableTransform().SetRotation(FQuat(newRotation));
				}

			}
		});
}

void USmartRotatingProcessor::ExecuteMovementRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	MovementRotatioQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& context)
		{
			const TArrayView<FSmartRotatingFragment> smartRotatingFragments{ context.GetMutableFragmentView<FSmartRotatingFragment>() };
			const TArrayView<FTransformFragment> transformFragments{ context.GetMutableFragmentView<FTransformFragment>() };
			const TConstArrayView<FMassVelocityFragment> velocityFragments{ context.GetFragmentView<FMassVelocityFragment>() };
			const FSmartRotatingSharedDataFragment& sharedDataFragment{ context.GetConstSharedFragment<FSmartRotatingSharedDataFragment>() };

			const float rotationSpeed{ sharedDataFragment.RotationSpeed };
			const float deltaTime{ context.GetDeltaTimeSeconds() };

			const int32 NumEntities{ context.GetNumEntities() };
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				FSmartRotatingFragment& smartRotatingFragment{ smartRotatingFragments[entityIndex] };
				FTransformFragment& transformFragment{ transformFragments[entityIndex] };
				const FMassVelocityFragment& velocityFragment{ velocityFragments[entityIndex] };

				FVector currentForward{ transformFragment.GetTransform().GetUnitAxis(EAxis::Y) };
				FVector targetDirection{ velocityFragment.Value.GetSafeNormal() };

				smartRotatingFragment.TargetDirection = targetDirection;

				if (!targetDirection.IsNearlyZero())
				{
					FVector newForward{ FMath::VInterpConstantTo(currentForward, targetDirection, deltaTime, rotationSpeed) };
					newForward.Normalize();

					const FRotator newRotation{ FRotationMatrix::MakeFromX(newForward).Rotator() };
					transformFragment.GetMutableTransform().SetRotation(FQuat(newRotation));
				}
			}
		});
}

void USmartRotatingProcessor::ExecuteTrackingRotation(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TrackingRotationQuery.ForEachEntityChunk(Context, [&EntityManager](FMassExecutionContext& context)
		{
			const TArrayView<FSmartRotatingFragment> smartRotatingFragments{ context.GetMutableFragmentView<FSmartRotatingFragment>() };
			const TArrayView<FTransformFragment> transformFragments{ context.GetMutableFragmentView<FTransformFragment>() };
			const FSmartRotatingSharedDataFragment& sharedDataFragment{ context.GetConstSharedFragment<FSmartRotatingSharedDataFragment>() };

			const float rotationSpeed{ sharedDataFragment.RotationSpeed };
			const float deltaTime{ context.GetDeltaTimeSeconds() };

			const int32 NumEntities{ context.GetNumEntities() };
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				FSmartRotatingFragment& smartRotFrag{ smartRotatingFragments[entityIndex] };
				FTransformFragment& transformFragment{ transformFragments[entityIndex] };

				FTransformFragment* targetTransformFragment{ EntityManager.GetFragmentDataPtr<FTransformFragment>(smartRotFrag.TargetEntity) };
				if (!targetTransformFragment) continue;

				if (!targetTransformFragment)
					continue;

				FTransform& transform{ transformFragment.GetMutableTransform() };

				const FVector start = transform.GetLocation();
				const FVector target = targetTransformFragment->GetTransform().GetLocation();
				const FVector targetDir = (target - start).GetSafeNormal();
				if (targetDir.IsNearlyZero())
					continue;

				// Desired world-space rotation
				FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(start, target);
				desiredRot.Yaw += smartRotFrag.LookAtOffset.X;
				desiredRot.Pitch += smartRotFrag.LookAtOffset.Y;
				desiredRot.Roll += smartRotFrag.LookAtOffset.Z;

				// Smooth interpolation
				const FQuat currentQuat = transform.GetRotation();
				const FQuat targetQuat = FQuat(desiredRot);
				const FQuat newQuat = FMath::QInterpConstantTo(currentQuat, targetQuat, deltaTime, rotationSpeed);

				transform.SetRotation(newQuat);
			}
		});
}

void USmartRotatingProcessor::ExecuteActorTrackingRotation(FMassEntityManager& EntityManager, FMassExecutionContext& context)
{
	ActorTrackingRotationQuery.ForEachEntityChunk(context, [&EntityManager](FMassExecutionContext& context)
		{

			const auto smartRotatingFragments{ context.GetMutableFragmentView<FSmartRotatingFragment>() };
			const auto smartRotActorFrag{ context.GetFragmentView<FSmartRotatingActorTrackingFragment>() };
			const auto transformFragments{ context.GetMutableFragmentView<FTransformFragment>() };
			const auto& sharedDataFragment{ context.GetConstSharedFragment<FSmartRotatingSharedDataFragment>() };

			const float rotationSpeed{ sharedDataFragment.RotationSpeed };
			const float deltaTime{ context.GetDeltaTimeSeconds() };

			const int32 NumEntities{ context.GetNumEntities() };
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				const FSmartRotatingActorTrackingFragment& actorTrackingFrag{ smartRotActorFrag[entityIndex] };
				if (actorTrackingFrag.TargetActor == nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("Entity %d has no TargetActor set for Actor Tracking Rotation."), context.GetEntity(entityIndex).Index);
					continue;
				}
					

				FSmartRotatingFragment& smartRotFrag{ smartRotatingFragments[entityIndex] };
				FTransformFragment& transformFragment{ transformFragments[entityIndex] };

				FTransform& transform{ transformFragment.GetMutableTransform() };

				const FVector start = transform.GetLocation();
				const FVector target = actorTrackingFrag.TargetActor->GetActorLocation();
				const FVector targetDir = (target - start).GetSafeNormal();

				UE_LOG(LogTemp, Warning, TEXT("Entity %d is tracking actor at location %s."), context.GetEntity(entityIndex).Index, *target.ToString());

				if (targetDir.IsNearlyZero())
					continue;

				// Desired world-space rotation
				FRotator desiredRot = UKismetMathLibrary::FindLookAtRotation(start, target);
				desiredRot.Yaw += smartRotFrag.LookAtOffset.X;
				desiredRot.Pitch += smartRotFrag.LookAtOffset.Y;
				desiredRot.Roll += smartRotFrag.LookAtOffset.Z;

				// Smooth interpolation
				const FQuat currentQuat = transform.GetRotation();
				const FQuat targetQuat = FQuat(desiredRot);
				const FQuat newQuat = FMath::QInterpConstantTo(currentQuat, targetQuat, deltaTime, rotationSpeed);

				transform.SetRotation(newQuat);
			}
		});
}



