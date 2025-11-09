#include "NPC/NPCCharacter.h"
#include "MassEntitySubsystem.h"
#include <MassStateTreeSubsystem.h>
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"
#include "MassEntityManager.h"

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

void ANPCCharacter::TalkToNPC()
{
	UWorld const* world = GetWorld();

	UMassSignalSubsystem* SignalSubsystem = world->GetSubsystem<UMassSignalSubsystem>();
	SignalSubsystem->SignalEntity(FName("TalkToNPC"), MassAgentComponent->GetEntityHandle());

	const UMassEntitySubsystem* EntitySubsystem = world->GetSubsystem<UMassEntitySubsystem>();
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();

	FMassStateTreeInstanceFragment& InstanceFragment = EntityManager.GetFragmentDataChecked<FMassStateTreeInstanceFragment>(MassAgentComponent->GetEntityHandle());

	UMassStateTreeSubsystem* StateTreeSubsystem = world->GetSubsystem<UMassStateTreeSubsystem>();
	StateTreeSubsystem->FreeInstanceData(InstanceFragment.InstanceHandle);
	UE_LOG(LogTemp, Warning, TEXT("NPC TalkToNPC triggered"));
	
}

