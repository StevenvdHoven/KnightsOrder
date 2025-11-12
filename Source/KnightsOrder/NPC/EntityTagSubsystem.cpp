#include "NPC/EntityTagSubsystem.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"


UEntityTagProcessor::UEntityTagProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Behavior);
	EntityQuery.RegisterWithProcessor(*this);
}

void UEntityTagProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FEntityTagFragment>(EMassFragmentAccess::ReadOnly);
}

void UEntityTagProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UEntityTagSubsystem* TagSubsystem = EntityManager.GetWorld()->GetSubsystem<UEntityTagSubsystem>();
	TagSubsystem->ClearCachedEntities();
	TMap<FName,TArray<FMassEntityHandle>> TaggedEntities;

	EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Context)
		{
			
			TConstArrayView<FEntityTagFragment> TagFragments = Context.GetFragmentView<FEntityTagFragment>();

			const int32 NumEntities = Context.GetNumEntities();
			for (int32 i = 0; i < NumEntities; ++i)
			{
				const FEntityTagFragment& TagFragment = TagFragments[i];
				const FMassEntityHandle Entity = Context.GetEntity(i);
				
				TaggedEntities.FindOrAdd(TagFragment.Tag).Add(Entity);
			}
		});

	for (const auto& Pair : TaggedEntities)
	{
		TagSubsystem->SetChachedEntities(Pair.Key, Pair.Value);
	}
}

void UEntityTagSubsystem::ClearCachedEntities()
{
	TaggedEntities.Empty();
}

void UEntityTagSubsystem::SetChachedEntities(const FName& tag, const TArray<FMassEntityHandle>& InTaggedEntities)
{
	for (auto entity : InTaggedEntities)
	{
		FTaggedEntity taggedEntity;
		taggedEntity.Tag = tag;
		taggedEntity.Entity = entity;
		TaggedEntities.Add(taggedEntity);
	}
}

TArray<FMassEntityHandle> UEntityTagSubsystem::GetCachedEntities(FName tag) const
{
	TArray<FMassEntityHandle> result;
	for (const FTaggedEntity& taggedEntity : TaggedEntities)
	{
		if (taggedEntity.Tag == tag)
		{
			result.Add(taggedEntity.Entity);
		}
	}
	return result;
	
}

bool UEntityTagSubsystem::TryFindEntityWithTag(FName Tag, FMassEntityHandle& outHandle) const
{
	for (FTaggedEntity taggedEntity : TaggedEntities)
	{
		if (taggedEntity.Tag == Tag)
		{
			outHandle = taggedEntity.Entity;
			return true;
		}
	}
	return false;
}




