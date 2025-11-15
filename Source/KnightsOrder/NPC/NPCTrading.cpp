#include "NPC/NPCTrading.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassNavigationSubsystem.h"
#include "MassNavigationProcessors.h"

constexpr int32 MaxExpectedAgentsPerCell = 6;
constexpr int32 MinTouchingCellCount = 4;
constexpr int32 MaxObstacleResults = MaxExpectedAgentsPerCell * MinTouchingCellCount;

static void FindCloseEntities(const FVector& Center, const FVector::FReal SearchRadius, const FNavigationObstacleHashGrid2D& AvoidanceObstacleGrid,
	TArray<FMassNavigationObstacleItem, TFixedAllocator<MaxObstacleResults>>& OutCloseEntities, const int32 MaxResults)
{
	OutCloseEntities.Reset();
	const FVector Extent(SearchRadius, SearchRadius, 0.);
	const FBox QueryBox = FBox(Center - Extent, Center + Extent);

	struct FSortingCell
	{
		int32 X;
		int32 Y;
		int32 Level;
		FVector::FReal SqDist;
	};
	TArray<FSortingCell, TInlineAllocator<64>> Cells;
	const FVector QueryCenter = QueryBox.GetCenter();

	for (int32 Level = 0; Level < AvoidanceObstacleGrid.NumLevels; Level++)
	{
		const FVector::FReal CellSize = AvoidanceObstacleGrid.GetCellSize(Level);
		const FNavigationObstacleHashGrid2D::FCellRect Rect = AvoidanceObstacleGrid.CalcQueryBounds(QueryBox, Level);
		for (int32 Y = Rect.MinY; Y <= Rect.MaxY; Y++)
		{
			for (int32 X = Rect.MinX; X <= Rect.MaxX; X++)
			{
				const FVector::FReal CenterX = (X + 0.5) * CellSize;
				const FVector::FReal CenterY = (Y + 0.5) * CellSize;
				const FVector::FReal DX = CenterX - QueryCenter.X;
				const FVector::FReal DY = CenterY - QueryCenter.Y;
				const FVector::FReal SqDist = DX * DX + DY * DY;
				FSortingCell SortCell;
				SortCell.X = X;
				SortCell.Y = Y;
				SortCell.Level = Level;
				SortCell.SqDist = SqDist;
				Cells.Add(SortCell);
			}
		}
	}

	Cells.Sort([](const FSortingCell& A, const FSortingCell& B) { return A.SqDist < B.SqDist; });

	for (const FSortingCell& SortedCell : Cells)
	{
		if (const FNavigationObstacleHashGrid2D::FCell* Cell = AvoidanceObstacleGrid.FindCell(SortedCell.X, SortedCell.Y, SortedCell.Level))
		{
			const TSparseArray<FNavigationObstacleHashGrid2D::FItem>& Items = AvoidanceObstacleGrid.GetItems();
			for (int32 Idx = Cell->First; Idx != INDEX_NONE; Idx = Items[Idx].Next)
			{
				OutCloseEntities.Add(Items[Idx].ID);
				if (OutCloseEntities.Num() >= MaxResults)
				{
					return;
				}
			}
		}
	}
}

void UNPCTradingTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FTradingFragment& TradingFragment = BuildContext.AddFragment_GetRef<FTradingFragment>();
	TradingFragment.DesiredProduct = DesiredProduct;
	TradingFragment.DesiredMinQuantity = DesiredMinQuantity;
	TradingFragment.DesiredMaxQuantity = DesiredMaxQuantity;

	for (auto& pair : StartingInventoryData)
	{
		const int32 randomAmount{ FMath::RandRange(pair.Value.MinQuantity, pair.Value.MaxQuantity) };
		TradingFragment.Inventory.Add(pair.Key, randomAmount);
	}

	FTradingProductionFragment& ProductionFragment = BuildContext.AddFragment_GetRef<FTradingProductionFragment>();
	ProductionFragment.ProducingProduct = ProducingProduct;

	BuildContext.AddFragment<FTradingRequestFragment>();
	BuildContext.AddFragment<FTradingPendingFragment>();

	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	const FConstSharedStruct sharedStruct{ EntityManager.GetOrCreateConstSharedFragment(SharedData) };
	BuildContext.AddConstSharedFragment(sharedStruct);

}

UNPCTradingProcessor::UNPCTradingProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Behavior);

	FindingTradingQuery.RegisterWithProcessor(*this);
	PendingTradingQuery.RegisterWithProcessor(*this);
	ProductionTradingQuery.RegisterWithProcessor(*this);
}

void UNPCTradingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FindingTradingQuery.AddRequirement<FTradingPendingFragment>(EMassFragmentAccess::ReadWrite);
	FindingTradingQuery.AddRequirement<FTradingRequestFragment>(EMassFragmentAccess::ReadWrite);
	FindingTradingQuery.AddRequirement<FTradingFragment>(EMassFragmentAccess::ReadOnly);
	FindingTradingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	FindingTradingQuery.AddConstSharedRequirement<FTradingSharedDataFragement>();
	FindingTradingQuery.AddTagRequirement<FTradingFindingTag>(EMassFragmentPresence::All);

	PendingTradingQuery.AddRequirement<FTradingPendingFragment>(EMassFragmentAccess::ReadOnly);
	PendingTradingQuery.AddRequirement<FTradingFragment>(EMassFragmentAccess::ReadOnly);
	PendingTradingQuery.AddRequirement<FTradingRequestFragment>(EMassFragmentAccess::ReadOnly);
	PendingTradingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	PendingTradingQuery.AddTagRequirement<FTradingPendingTag>(EMassFragmentPresence::All);

	ProductionTradingQuery.AddRequirement<FTradingProductionFragment>(EMassFragmentAccess::ReadWrite);
}

void UNPCTradingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ProcessFindingTrading(EntityManager, Context);
	ProcessPendingTrading(EntityManager, Context);
	ProcessProductionTrading(EntityManager, Context);
}

void UNPCTradingProcessor::ProcessFindingTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UNPCTradingSubSystem* TradingSubSystem = Context.GetWorld()->GetSubsystem<UNPCTradingSubSystem>();

	const float deltaTime{ Context.GetDeltaTimeSeconds() };
	FindingTradingQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Context)
		{
			const TArrayView<FTradingPendingFragment> TradingRequests{ Context.GetMutableFragmentView<FTradingPendingFragment>() };
			const TArrayView<FTradingRequestFragment> RequestFragments{ Context.GetMutableFragmentView<FTradingRequestFragment>() };
			const TConstArrayView<FTradingFragment> TradingFragments{ Context.GetFragmentView<FTradingFragment>() };
			const TConstArrayView<FTransformFragment> TransformFragments{ Context.GetFragmentView<FTransformFragment>() };
			const FTradingSharedDataFragement& SharedData{ Context.GetConstSharedFragment<FTradingSharedDataFragement>() };

			const int32 NumEntities{ Context.GetNumEntities() };
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				auto EntityHandle{ Context.GetEntity(entityIndex) };
				FTradingRequestFragment& requestFragment{ RequestFragments[entityIndex] };

				TArray<FMassEntityHandle> CloseEntities;
				const FVector& EntityLocation{ TransformFragments[entityIndex].GetTransform().GetLocation() };
				CloseEntities = TradingSubSystem->GetCloseTradingEntities(EntityLocation, SharedData.TradingDistance);
				UE_LOG(LogTemp, Log, TEXT("Entity %d found %d close entities"), EntityHandle.Index, CloseEntities.Num());

				TArray<FTradingEntry> TradeEntries;
				ExtractTradeEntries(CloseEntities, TradeEntries);

				FTradingEntry optimalTrade{ EntityHandle, TradingFragments[entityIndex], ETradingProduct::Wood, 0, 0 };
				if (FindOptimalTradingOption(RequestFragments[entityIndex], TradeEntries, optimalTrade))
				{
					UE_LOG(LogTemp, Log, TEXT("Entity %d found optimal trade with entity %d for product %d, quantity %d, cost %d"),
						EntityHandle.Index, optimalTrade.EntityHandle.Index, (int)optimalTrade.SellingProduct, optimalTrade.Quantity, optimalTrade.Cost);
					FTradingPendingFragment& tradingPendingFragment = TradingRequests[entityIndex];
					tradingPendingFragment.TradingPartner = optimalTrade.EntityHandle;
					tradingPendingFragment.ProductToTrade = optimalTrade.SellingProduct;
					tradingPendingFragment.QuantityToTrade = optimalTrade.Quantity;
					tradingPendingFragment.bIsBuyer = true;

					Context.Defer().RemoveTag<FTradingFindingTag>(EntityHandle);
					Context.Defer().AddTag<FTradingPendingTag>(EntityHandle);

					FTradingPendingFragment& partnerTradingPendingFragment = EntityManager.GetFragmentDataChecked<FTradingPendingFragment>(optimalTrade.EntityHandle);
					partnerTradingPendingFragment.TradingPartner = EntityHandle;
					partnerTradingPendingFragment.ProductToTrade = RequestFragments[entityIndex].RequestedProduct;
					partnerTradingPendingFragment.QuantityToTrade = RequestFragments[entityIndex].RequestedQuantity;
					partnerTradingPendingFragment.bIsBuyer = false;

					Context.Defer().AddTag<FTradingPendingTag>(optimalTrade.EntityHandle);
					requestFragment.RequestStatus = ETradingStatus::Completed;
				}

				requestFragment.TimeSinceRequest += deltaTime;
				if (requestFragment.TimeSinceRequest >= SharedData.TradingDuration)
				{
					requestFragment.RequestStatus = ETradingStatus::Failed;
					Context.Defer().RemoveTag<FTradingFindingTag>(EntityHandle);
					UE_LOG(LogTemp, Log, TEXT("Entity %d trading request timed out"), EntityHandle.Index);
				}
				
			}
		});
}

void UNPCTradingProcessor::ProcessPendingTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	PendingTradingQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Context)
		{
			const TConstArrayView<FTradingFragment> TradingFragments{ Context.GetMutableFragmentView<FTradingFragment>() };
			const TConstArrayView<FTradingPendingFragment> TradingRequests{ Context.GetMutableFragmentView<FTradingPendingFragment>() };
			const TConstArrayView<FTransformFragment> TransformFragments{ Context.GetFragmentView<FTransformFragment>() };
			const FTradingSharedDataFragement& SharedData{ Context.GetConstSharedFragment<FTradingSharedDataFragement>() };

			const int32 NumEntities{ Context.GetNumEntities() };
			for (int32 entityIndex{ 0 }; entityIndex < NumEntities; ++entityIndex)
			{
				auto entityHandle{ Context.GetEntity(entityIndex) };
				const FTradingPendingFragment& tradingRequest{ TradingRequests[entityIndex] };
				if (!tradingRequest.bIsBuyer) 
				{
					// Set navigation to not moving waiting for the buyer to arrive
				}
				else 
				{
					// Set navigation to move to the seller's location
				}
			}

		});
}

void UNPCTradingProcessor::ProcessProductionTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ProductionTradingQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& Context)
		{
			// Processing logic for production trading entities would go here
		});
}

void UNPCTradingProcessor::ExtractTradeEntries(const TArray<FMassEntityHandle>& entities, TArray<FTradingEntry>& outEntries)
{
	UNPCTradingSubSystem* TradingSubSystem = GetWorld()->GetSubsystem<UNPCTradingSubSystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	for (auto& entityHandle : entities)
	{
		FTradingFragment* tradingFragment{ EntityManager.GetFragmentDataPtr<FTradingFragment>(entityHandle) };
		if (tradingFragment == nullptr)
			continue;

		for (auto& item : tradingFragment->Inventory)
		{
			ETradingProduct productType = item.Key;
			int32 quantity = item.Value;
			int32 productValue = TradingSubSystem->GetTradingProductValue(productType);
			int32 totalCost = productValue * quantity;

			FTradingEntry entry{ entityHandle, *tradingFragment, productType, quantity, totalCost };
			outEntries.Add(entry);
		}
	}
}

bool UNPCTradingProcessor::FindOptimalTradingOption(const FTradingRequestFragment& requestingTrader, const TArray<FTradingEntry>& optionalTrades, FTradingEntry& output)
{
	UNPCTradingSubSystem* TradingSubSystem = GetWorld()->GetSubsystem<UNPCTradingSubSystem>();

	TArray<FTradingEntry> payableTrades;
	int32 totalBudget{ requestingTrader.RequestedQuantity * TradingSubSystem->GetTradingProductValue(requestingTrader.RequestedProduct) };

	for (auto& entry : optionalTrades)
	{
		if (entry.SellingProduct != requestingTrader.RequestedProduct)
			continue;

		if (entry.Cost <= totalBudget)
		{
			payableTrades.Add(entry);
		}
	}

	payableTrades.Sort([](const FTradingEntry& A, const FTradingEntry& B)
		{
			return A.Cost > B.Cost;
		});

	if (payableTrades.Num() > 0)
	{
		FTradingEntry& foundTrade = payableTrades[0];
		output.EntityHandle = foundTrade.EntityHandle;
		output.TradingFragment = foundTrade.TradingFragment;
		output.SellingProduct = foundTrade.SellingProduct;
		output.Quantity = foundTrade.Quantity;
		output.Cost = foundTrade.Cost;
		return true;
	}
	return false;
}

void UNPCTradingSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TradingProductValues.Add(ETradingProduct::Wood, 10);
	TradingProductValues.Add(ETradingProduct::Stone, 15);
	TradingProductValues.Add(ETradingProduct::Iron, 25);

	UE_LOG(LogTemp, Log, TEXT("NPC Trading Subsystem Initialized"));

	WorldPtr = GetWorld();
}

void UNPCTradingSubSystem::SetTradingProductValue(const ETradingProduct& productType, int32 value)
{
	if (TradingProductValues.Contains(productType))
		TradingProductValues[productType] = value;
	else
		TradingProductValues.Add(productType, value);

	UE_LOG(LogTemp, Log, TEXT("Set trading product %d value to %d"), (int)productType, value);
}

void UNPCTradingSubSystem::SetTradingRequest(const FMassEntityHandle& EntityHandle, const ETradingProduct& DesiredProduct, int32 DesiredQuantity)
{
	auto& EntityManager = WorldPtr->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
	FTradingRequestFragment& TradingRequestFragment = EntityManager.GetFragmentDataChecked<FTradingRequestFragment>(EntityHandle);
	TradingRequestFragment.RequestedProduct = DesiredProduct;
	TradingRequestFragment.RequestedQuantity = DesiredQuantity;
	TradingRequestFragment.RequestStatus = ETradingStatus::Pending;

	UE_LOG(LogTemp, Log, TEXT("Set trading request for entity %d: product %d, quantity %d"), EntityHandle.Index, (int)DesiredProduct, DesiredQuantity);

	EntityManager.Defer().AddTag<FTradingFindingTag>(EntityHandle);
	ActiveTradingEntities.Add(EntityHandle);
}

bool UNPCTradingSubSystem::HasPendingTradingRequest(const FMassEntityHandle& EntityHandle) const
{
	auto& EntityManager = WorldPtr->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();
	if (ActiveTradingEntities.Contains(EntityHandle))
	{
		return true;
	}
	return false;
	
}

int32 UNPCTradingSubSystem::GetTradingProductValue(const ETradingProduct& productType) const
{
	if (TradingProductValues.Contains(productType))
	{
		return TradingProductValues[productType];
	}
	return int32();
}

TArray<FMassEntityHandle> UNPCTradingSubSystem::GetCloseTradingEntities(const FVector& Location, float Radius) const
{
	UMassNavigationSubsystem* NavigationSubsystem = WorldPtr->GetSubsystem<UMassNavigationSubsystem>();
	TArray<FMassNavigationObstacleItem, TFixedAllocator<MaxObstacleResults>> CloseEntities;

	auto grid = NavigationSubsystem->GetObstacleGridMutable();
	FindCloseEntities(Location, Radius, grid, CloseEntities, MaxObstacleResults);

	TArray<FMassEntityHandle> ResultEntities;
	ResultEntities.Reserve(CloseEntities.Num());
	for (auto& ObstacleItem : CloseEntities)
	{
		ResultEntities.Add(ObstacleItem.Entity);
	}
	
	return ResultEntities;

}


