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

void ANPCCharacter::TalkToNPC(const FVector& lookAt)
{
	UWorld const* world = GetWorld();

	UMassSignalSubsystem* SignalSubsystem = world->GetSubsystem<UMassSignalSubsystem>();
	SignalSubsystem->SignalEntity(FName("TalkToNPC"), MassAgentComponent->GetEntityHandle());

	const UMassEntitySubsystem* EntitySubsystem = world->GetSubsystem<UMassEntitySubsystem>();
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	UEntityTagSubsystem* TagSubsystem = world->GetSubsystem<UEntityTagSubsystem>();

	auto foundPlayers = TagSubsystem->GetCachedEntities(FName("Player"));
	


	FMassStateTreeInstanceFragment& InstanceFragment = EntityManager.GetFragmentDataChecked<FMassStateTreeInstanceFragment>(MassAgentComponent->GetEntityHandle());

	UMassStateTreeSubsystem* StateTreeSubsystem = world->GetSubsystem<UMassStateTreeSubsystem>();
	StateTreeSubsystem->FreeInstanceData(InstanceFragment.InstanceHandle);
	UE_LOG(LogTemp, Warning, TEXT("NPC TalkToNPC triggered"));

	FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(MassAgentComponent->GetEntityHandle());
	MoveTarget.DesiredSpeed = FMassInt16Real{ 0.0f };
	MoveTarget.IntentAtGoal = EMassMovementAction::Animate;

	if (foundPlayers.Num() > 0)
	{
		UMassLookAtSubsystem* LookAtSubsystem = world->GetSubsystem<UMassLookAtSubsystem>();
		FMassLookAtFragment& LookAt = EntityManager.GetFragmentDataChecked<FMassLookAtFragment>(MassAgentComponent->GetEntityHandle());
		LookAt.OverrideState = FMassLookAtFragment::EOverrideState::ActiveOverrideOnly;
		LookAt.LookAtMode = EMassLookAtMode::LookAtEntity;
		LookAt.TrackedEntity = foundPlayers[0];
	}

}

