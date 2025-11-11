#include "NPC/NPCCharacter.h"
#include "MassEntitySubsystem.h"
#include <MassStateTreeSubsystem.h>
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"
#include "MassEntityManager.h"
#include "MassNavigationFragments.h"
#include "MassLookAtFragments.h"
#include "NPC/EntityTagSubsystem.h"
#include "MassLookAtSubsystem.h"
#include "MassLookAtTypes.h"
#include "SmartRotating.h"
#include "Kismet/KismetMathLibrary.h"

ANPCCharacter::ANPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	MassAgentComponent = FindComponentByClass<UMassAgentComponent>();
}

void ANPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANPCCharacter::TalkToNPC(const FVector& lookAt, AActor* interactingActor)
{
	UWorld const* world = GetWorld();
	const FMassEntityHandle& entityHandle = MassAgentComponent->GetEntityHandle();

	UMassSignalSubsystem* SignalSubsystem = world->GetSubsystem<UMassSignalSubsystem>();
	SignalSubsystem->SignalEntity(FName("TalkToNPC"), entityHandle);

	const UMassEntitySubsystem* EntitySubsystem = world->GetSubsystem<UMassEntitySubsystem>();
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	UEntityTagSubsystem* TagSubsystem = world->GetSubsystem<UEntityTagSubsystem>();

	auto foundPlayers = TagSubsystem->GetCachedEntities(FName("Player"));
	


	FMassStateTreeInstanceFragment& InstanceFragment = EntityManager.GetFragmentDataChecked<FMassStateTreeInstanceFragment>(entityHandle);

	UMassStateTreeSubsystem* StateTreeSubsystem = world->GetSubsystem<UMassStateTreeSubsystem>();
	StateTreeSubsystem->FreeInstanceData(InstanceFragment.InstanceHandle);
	UE_LOG(LogTemp, Warning, TEXT("NPC TalkToNPC triggered"));

	FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(entityHandle);
	MoveTarget.DesiredSpeed = FMassInt16Real{ 0.0f };
	MoveTarget.IntentAtGoal = EMassMovementAction::Animate;

	if (foundPlayers.Num() > 0)
	{
		FSmartRotatingFragment* smartRotatingFragment{ EntityManager.GetFragmentDataPtr<FSmartRotatingFragment>(entityHandle) };
		FSmartRotatingActorTrackingFragment* smartRotActorFrag{ EntityManager.GetFragmentDataPtr<FSmartRotatingActorTrackingFragment>(entityHandle) };
		FTransformFragment* playerTransformFragment{ EntityManager.GetFragmentDataPtr<FTransformFragment>(foundPlayers[0]) };

		smartRotatingFragment->TargetEntity = foundPlayers[0];
		smartRotatingFragment->LookAtOffset = LookAtOffset;
		smartRotatingFragment->TargetDirection = playerTransformFragment->GetTransform().GetLocation() - GetActorLocation();
		smartRotatingFragment->TargetDirection.Z = 0.0f;

		smartRotActorFrag->TargetActor = interactingActor;

		UE_LOG(LogTemp, Log, TEXT("NPC LookAt Player Entity"));

		EntityManager.Defer().RemoveTag<FSmartRotatedWithMovementTag>(entityHandle);
		EntityManager.Defer().AddTag<FSmartTrackingActorTag>(entityHandle);
	}

}

