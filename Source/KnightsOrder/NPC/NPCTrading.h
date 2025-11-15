#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTraitBase.h"
#include "NPCTrading.generated.h"

UENUM(BlueprintType)
enum struct ETradingProduct : uint8
{
	None UMETA(DisplayName = "None"),
	Wood UMETA(DisplayName = "Wood"),
	Stone UMETA(DisplayName = "Stone"),
	Iron UMETA(DisplayName = "Iron"),
	
};

USTRUCT(BlueprintType)
struct FTradingFindingTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FTradingPendingTag : public FMassTag
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FTradingFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ETradingProduct, int32> Inventory;

	UPROPERTY()
	ETradingProduct DesiredProduct;

	UPROPERTY()
	int32 DesiredMinQuantity;

	UPROPERTY()
	int32 DesiredMaxQuantity;

	UPROPERTY()
	bool bIsMovingToTrade;
};

UENUM(BlueprintType)
enum struct ETradingStatus : uint8
{
	Pending UMETA(DisplayName = "Pending"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed"),
};

USTRUCT(BlueprintType)
struct FTradingRequestFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	ETradingProduct RequestedProduct;

	UPROPERTY()
	int32 RequestedQuantity;

	UPROPERTY()
	ETradingStatus RequestStatus;

	UPROPERTY()
	float TimeSinceRequest;
};

USTRUCT(BlueprintType)
struct FTradingPendingFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FMassEntityHandle TradingPartner;

	UPROPERTY()
	ETradingProduct ProductToTrade;

	UPROPERTY()
	int32 QuantityToTrade;

	UPROPERTY()
	bool bIsBuyer;

	UPROPERTY()
	ETradingStatus TradeStatus;

};

USTRUCT(BlueprintType)
struct FTradingProductionFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	ETradingProduct ProducingProduct;

	UPROPERTY()
	float TradingTimeElapsed;
};

USTRUCT(BlueprintType)
struct FTradingSharedDataFragement : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float TradingDuration;

	UPROPERTY(EditAnywhere)
	float TradingDistance;
};

USTRUCT(BlueprintType)
struct FTradeProductStartData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 MinQuantity;

	UPROPERTY(EditAnywhere)
	int32 MaxQuantity;
};

UCLASS(BlueprintType)
class UNPCTradingTrait :public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override final;


private:
	UPROPERTY(EditAnywhere)
	ETradingProduct ProducingProduct;

	UPROPERTY(EditAnywhere)
	ETradingProduct DesiredProduct;

	UPROPERTY(EditAnywhere)
	int32 DesiredMinQuantity;

	UPROPERTY(EditAnywhere)
	int32 DesiredMaxQuantity;

	UPROPERTY(EditAnywhere)
	TMap<ETradingProduct, FTradeProductStartData> StartingInventoryData;

	UPROPERTY(EditAnywhere)
	FTradingSharedDataFragement SharedData;
};

UCLASS()
class KNIGHTSORDER_API UNPCTradingProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	UNPCTradingProcessor();

protected:
	void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
private:
	void ProcessFindingTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	void ProcessPendingTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
	void ProcessProductionTrading(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

	struct FTradingEntry
	{
	public:
		FMassEntityHandle EntityHandle;
		FTradingFragment TradingFragment;
		ETradingProduct SellingProduct;
		int32 Quantity;
		int32 Cost;
	};

	
	void ExtractTradeEntries(const TArray<FMassEntityHandle>& entities, TArray<FTradingEntry>& outEntries);
	bool FindOptimalTradingOption(const FTradingRequestFragment& requestingTrader, const TArray<FTradingEntry>& optionalTrades, FTradingEntry& output);

	FMassEntityQuery FindingTradingQuery;
	FMassEntityQuery PendingTradingQuery;
	FMassEntityQuery ProductionTradingQuery;
};

UCLASS()
class KNIGHTSORDER_API UNPCTradingSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:	
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void SetTradingProductValue(const ETradingProduct& productType, int32 value);

	void SetTradingRequest(const FMassEntityHandle& EntityHandle, const ETradingProduct& DesiredProduct, int32 DesiredQuantity);

	bool HasPendingTradingRequest(const FMassEntityHandle& EntityHandle) const;
	int32 GetTradingProductValue(const ETradingProduct& productType) const;

	TArray<FMassEntityHandle> GetCloseTradingEntities(const FVector& Location, float Radius) const;

private:
	UPROPERTY()
	TObjectPtr<UWorld> WorldPtr;

	UPROPERTY()
	TMap<ETradingProduct, int32> TradingProductValues;

	UPROPERTY()
	TMap<ETradingProduct, float> TradingProductionRate;

	UPROPERTY()
	TArray<FMassEntityHandle> ActiveTradingEntities;
};
