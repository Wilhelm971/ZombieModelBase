// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include <vector>
#include "SimulationController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayAdvanced, int32, Day);


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

// Struct for Conveyor Batches
USTRUCT(BlueprintType)
struct FConveyorBatch
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float AmountOfPeople = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float RemainingDays = 0.f;
};

UCLASS()
class ZOMBIEAPOCALYPSE_API ASimulationController : public AActor
{
    GENERATED_BODY()

public:
    ASimulationController();

    // Runs one simulation step each Tick (but we'll override for turn-based)
    virtual void Tick(float DeltaTime) override;

    // Function to read data from Unreal DataTable into the graphPts vector
    void ReadDataFromTableToVectors();

    // Advance the simulation by one day (called on player "End Turn")
    UFUNCTION(BlueprintCallable, Category = "Simulation")
    void AdvanceOneDay();

    // Place a fence (player action) - reduces containment multiplier
    UFUNCTION(BlueprintCallable, Category = "Simulation")
    void PlaceFence(int32 EdgeID);

    // Get current bitten for HUD
    UFUNCTION(BlueprintPure, Category = "Simulation")
    float GetCurrentBitten() const { return CurrentBitten; }

    // Unreal Lookup table for population density effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    class UDataTable* PopulationDensityEffectTable{ nullptr };

    // Turn on/off debug printing to the Output Log
    UPROPERTY(EditAnywhere, Category = "Simulation Variables")
    bool bShouldDebug{ false };

    // Simulation constants
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float DaysToBecomeZombie = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float NormalBiteRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float LandArea = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float NormalPopulationDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float BittenCapacity = 100.f;

    // Initial stocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    float InitialSusceptible = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    float InitialZombies = 1.f;

    // Live stocks (exposed for HUD/Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    float CurrentSusceptible;

    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    float CurrentZombies;

    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    float CurrentBitten = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    int32 CurrentDay = 0;

    // Containment effect from fences (1.0 = no effect, lower = fewer bites)
    UPROPERTY(BlueprintReadOnly, Category = "Containment")
    float ContainmentEffectMultiplier = 1.0f;

    // Delegate for day advance (optional, for UI updates)
    UPROPERTY(BlueprintAssignable, Category = "Simulation")
    FOnDayAdvanced OnDayAdvanced;

    // Simple grid for fences (e.g., 10x10 square grid, edges between cells)
    // For demo: assume 100 edges, track which are fenced
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Containment")
    int32 MaxEdges = 0;

    // Grid settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeX = 5;  // Number of cells X

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeY = 5;  // Number of cells Y

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSpacing = 100.f;  // Distance between cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<class AEdgeActor> EdgeActorClass;  // Assign in Editor

    // Spawn grid function
    void SpawnGrid();


    // Cell actor class for visuals
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<class ACellActor> CellActorClass;  // Assign in Editor

    // Array of spawned cell actors
    TArray<ACellActor*> CellActors;

    // Update visuals after simulation step
    void UpdateCellVisuals();

protected:
    virtual void BeginPlay() override;

private:
    // GRAPH points: population_density_effect_on_zombie_bites
    TArray<std::pair<float, float>> graphPts;

    // Conveyor for bitten (incubating)
    TArray<FConveyorBatch> BittenConveyor;

    // Graph lookup function
    float GraphLookup(float X) const;

    // Fenced edges (bit array for simplicity)
    TBitArray<FDefaultAllocator> FencedEdges;

    // Calculate containment based on fenced edges (e.g., % closed)
    void UpdateContainmentEffect();
};
