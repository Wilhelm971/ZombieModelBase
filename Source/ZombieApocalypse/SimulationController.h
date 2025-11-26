// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include <vector>
#include "GridManager.h"
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
    int AmountOfPeople = 0;
    float RemainingDays = 0.f;
    int LocationX = 0;
    int LocationY = 0;
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
    int Susceptible{ 100 };  // <- FIX: int, not float

    int Zombies{ 1 };  // <- FIX: int
    int Bitten{ 0 };  // <- FIX: int

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    class UDataTable* PopulationDensityEffectTable{ nullptr };

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    float SimulationStepTime{ 1.f };

    UPROPERTY(EditAnywhere, Category = "Simulation Variables")
    bool bShouldDebug{ false };

    /*=== Simulation constants ===*/
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

    // New: Turn-based control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    bool bIsPlayerTurn{ true };

    // New: Called by player controller to advance
    UFUNCTION(BlueprintCallable)
    void EndPlayerTurn();

protected:
    virtual void BeginPlay() override;

private:
    class AGridManager* GridManager{ nullptr };

    // Helpers
    void ReadDataFromTableToVectors();
    float GraphLookup(float X) const;
    int ConveyorContent() const;
    void PerformSimulationStep();
    void UpdateGlobalCounts(); // New: Recalculate Susceptible/Bitten/Zombies from grid
};