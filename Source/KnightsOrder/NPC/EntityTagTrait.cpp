#include "NPC/EntityTagTrait.h"
#include "MassEntityTemplateRegistry.h"

void UEntityTagTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FEntityTagFragment& TagFragment = BuildContext.AddFragment_GetRef<FEntityTagFragment>();
	TagFragment.Tag = DefaultTag;
}
