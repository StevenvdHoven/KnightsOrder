#include "NPC/TradingTasks.h"
#include "StateTreeExecutionContext.h"
#include "MassStateTreeExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "GameplayTagsManager.h"

EStateTreeRunStatus FSearchForTrade::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassContext{static_cast<const FMassStateTreeExecutionContext&>(Context)};
	FSearchForTrade& InstanceData{ Context.GetInstanceData<FSearchForTrade>(*this) };
	UNPCTradingSubSystem* TradingSubsystem{ Context.GetWorld()->GetSubsystem<UNPCTradingSubSystem>() };

	const int32 randomQuantity{ FMath::RandRange(InstanceData.MinimumQuantity, InstanceData.MaximumQuantity) };

	TradingSubsystem->SetTradingRequest(MassContext.GetEntity(),DesiredProduct, randomQuantity);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSearchForTrade::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassContext{ static_cast<const FMassStateTreeExecutionContext&>(Context) };
	FMassEntityManager& EntityManager{ MassContext.GetEntityManager() };

	FTradingRequestFragment& tradingRequest{ EntityManager.GetFragmentDataChecked<FTradingRequestFragment>(MassContext.GetEntity()) };
	if (tradingRequest.RequestStatus == ETradingStatus::Completed)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	else if (tradingRequest.RequestStatus == ETradingStatus::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FFindPositionToTrade::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassStateTreeExecutionContext& MassContext{ static_cast<const FMassStateTreeExecutionContext&>(Context) };
	FMassEntityManager& EntityManager{ MassContext.GetEntityManager() };

	FFindPositionToTrade& InstanceData{ Context.GetInstanceData<FFindPositionToTrade>(*this) };
	FTradingPendingFragment& tradingPending{ EntityManager.GetFragmentDataChecked<FTradingPendingFragment>(MassContext.GetEntity()) };
	
	if (!tradingPending.bIsBuyer)
	{
		FWaitForTradeEvent waitEvent;
		waitEvent.Tag = UGameplayTagsManager::Get().RequestGameplayTag("Trade.Wait");

		Context.SendEvent(waitEvent.Tag);
		return EStateTreeRunStatus::Running;
	}

	FTransformFragment& transformFragment{ EntityManager.GetFragmentDataChecked<FTransformFragment>(tradingPending.TradingPartner) };

	InstanceData.OutputLocation.EndOfPathPosition = transformFragment.GetTransform().GetLocation();
	InstanceData.OutputLocation.EndOfPathIntent = EMassMovementAction::Move;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FWaitForTrade::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassContext{ static_cast<const FMassStateTreeExecutionContext&>(Context) };
	FMassEntityManager& EntityManager{ MassContext.GetEntityManager() };

	FTradingPendingFragment& tradingPending{ EntityManager.GetFragmentDataChecked<FTradingPendingFragment>(MassContext.GetEntity()) };
	if (tradingPending.TradeStatus == ETradingStatus::Completed)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	else if (tradingPending.TradeStatus == ETradingStatus::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}

	return EStateTreeRunStatus::Running;
}
