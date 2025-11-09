#include "NPC/AdvancedMovementTrait.h"
#include "MassEntityTemplateRegistry.h"

void UAdvancedMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FAdvancedMovementFragment& advancedMovement = BuildContext.AddFragment_GetRef<FAdvancedMovementFragment>();
	advancedMovement.bIsMoving = false;
	advancedMovement.Speed = 0.0f;
	advancedMovement.MaximumSpeed = DefaultMaximumSpeed;
	advancedMovement.Acceleration = DefaultAcceleration;
	advancedMovement.BrakingDistance = DefaultBrakingDistance;
	advancedMovement.Deceleration = DefaultDeceleration;

}
