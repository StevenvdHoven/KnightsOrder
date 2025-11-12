#include "NPC/EntitySimpleDialogues.h"
#include "MassEntityTemplateRegistry.h"
#include "MassEntitySubsystem.h"

void UEntitySimpleDialoguesTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	auto& fragment{ BuildContext.AddFragment_GetRef<FEntitySimpleDialoguesFragment>() };
	fragment.bIsTalking = false;
	fragment.SpeakerName = SpeakerNames.Num() > 0 ? SpeakerNames[FMath::RandRange(0, SpeakerNames.Num() - 1)] : FName("NPC");
	fragment.AvailableDialogues.Empty();

	int dialoguesToAssign = FMath::Min(MaxDialoguesPerEntity, DialoguesOptions.Num());
	TSet<int> selectedIndices;
	while (fragment.AvailableDialogues.Num() < dialoguesToAssign)
	{
		int randomIndex = FMath::RandRange(0, DialoguesOptions.Num() - 1);
		if (!selectedIndices.Contains(randomIndex))
		{
			selectedIndices.Add(randomIndex);
			fragment.AvailableDialogues.Add(DialoguesOptions[randomIndex]);
		}
	}
}

const FEntitySimpleDialoguesFragment& UEntitySimpleDialoguesSubSystem::GetDialoguesFragment(const FMassEntityHandle& EntityHandle) const
{
	UWorld const* world = GetWorld();

	const UMassEntitySubsystem* EntitySubsystem = world->GetSubsystem<UMassEntitySubsystem>();
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();

	return EntityManager.GetFragmentDataChecked<FEntitySimpleDialoguesFragment>(EntityHandle);
}
