#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTraitBase.h"
#include "EntitySimpleDialogues.generated.h"

USTRUCT(BlueprintType)
struct FEntitySimpleDialoguesFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsTalking;

	UPROPERTY()
	FName SpeakerName;

	UPROPERTY()
	TArray<FText> AvailableDialogues;
};

UCLASS()
class UEntitySimpleDialoguesTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override final;

private:
	UPROPERTY(EditAnywhere)
	TArray<FName> SpeakerNames;

	UPROPERTY(EditAnywhere)
	int MaxDialoguesPerEntity;

	UPROPERTY(EditAnywhere)
	TArray<FText> DialoguesOptions;
};



UCLASS()
class KNIGHTSORDER_API UEntitySimpleDialoguesSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UEntitySimpleDialoguesSubSystem() {};

	const FEntitySimpleDialoguesFragment& GetDialoguesFragment(const FMassEntityHandle& EntityHandle) const;
	
};
