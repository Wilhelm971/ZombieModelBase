// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include <vector>
#include "SimulationController.generated.h"


// Struct for the Unreal DataTable
USTRUCT(BlueprintType)
struct FPopulationDensityEffect : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PopulationDensity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NormalPopulationDensity;
};

struct FConveyorBatch
{
	float AmountOfPeople = 0.f;
	float RemainingDays = 0.f;
};


UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationController : public AActor
{
	GENERATED_BODY()
	
public:	
	ASimulationController();
	virtual void Tick(float DeltaTime) override;


	/*=== Public for HUD ===*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	float Susceptible{ 100.f };

	float Zombies{ 1.f };
	float Bitten{ 0.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	class UDataTable* PopulationDensityEffectTable{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
	float SimulationStepTime{ 1.f };

	UPROPERTY(EditAnywhere, Category = "Simulation Variables")
	bool bShouldDebug{ false };


	/*=== simulation constants ===*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
	float DaysToBecomeInfectedFromBite{ 15.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
	float BittenCapacity{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
	float NormalNumberOfBites{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
	float LandArea{ 1000.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
	float NormalPopulationDensity{ 0.1f };


	/*=== Runtime data ===*/
	std::vector<std::pair<float, float>> graphPts;
	std::vector<FConveyorBatch> Conveyor;
	float AccumulatedTime{ 0.f };
	int TimeStepsFinished{ 0 };

protected:
	virtual void BeginPlay() override;

private:
	// Helpers
	void ReadDataFromTableToVectors();
	float GraphLookup(float X) const;
	float ConveyorContent() const;
	void PerformSimulationStep();
};