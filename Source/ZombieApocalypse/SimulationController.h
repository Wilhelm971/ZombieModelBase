// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "CellActor.h"
#include "EdgeActor.h"
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

// Enum for cell occupant
enum class ECellType : uint8
{
    Empty,
    Human,
    Zombie
};

// Struct for bitten off-map
struct FBitten
{
    int32 RemainingDays = 0;
};

// Neighbor direction struct
struct FNeighbor
{
    int32 DX;
    int32 DY;
};

// Cell position
struct FCellPos
{
    int32 X = 0;
    int32 Y = 0;

    bool operator==(const FCellPos& Other) const
    {
        return X == Other.X && Y == Other.Y;
    }

    friend FORCEINLINE uint32 GetTypeHash(const FCellPos& Key)
    {
        return GetTypeHash(Key.X) ^ GetTypeHash(Key.Y);
    }
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
    float GetCurrentBitten() const { return Bitten.Num(); }  // Number of bitten

    // Unreal Lookup table for population density effect
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    class UDataTable* PopulationDensityEffectTable{ nullptr };

    // Turn on/off debug printing to the Output Log
    UPROPERTY(EditAnywhere, Category = "Simulation Variables")
    bool bShouldDebug{ false };

    // Simulation constants
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Constants")
    float DaysToBecomeZombie = 15.f;

    // Initial stocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    int32 InitialHumans = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Variables")
    int32 InitialZombies = 1;

    // Live stocks (exposed for HUD/Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    int32 CurrentHumans;

    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    int32 CurrentZombies;

    UPROPERTY(BlueprintReadOnly, Category = "Simulation")
    int32 CurrentDay = 0;

    // Delegate for day advance (optional, for UI updates)
    UPROPERTY(BlueprintAssignable, Category = "Simulation")
    FOnDayAdvanced OnDayAdvanced;

    // Simple grid for fences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Containment")
    int32 MaxEdges = 0;

    // Grid settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeX = 10;  // Larger grid to fit individuals

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 GridSizeY = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    float CellSpacing = 100.f;  // Distance between cells

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<AEdgeActor> EdgeActorClass;  // Assign in Editor

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TSubclassOf<ACellActor> CellActorClass;  // Assign in Editor

    // Spawn grid function
    void SpawnGrid();

protected:
    virtual void BeginPlay() override;

private:
    // GRAPH points: population_density_effect_on_zombie_bites (kept if needed for future)
    TArray<std::pair<float, float>> graphPts;

    // Fenced edges (bit array for simplicity)
    TBitArray<> FencedEdges;

    // Grid occupants (2D array)
    TArray<TArray<ECellType>> GridOccupants;

    // List of bitten off-map
    TArray<FBitten> Bitten;

    // Array of spawned cell actors
    TArray<ACellActor*> CellActors;

    // Directions for neighbors
    TArray<FNeighbor> Directions;

    // Update cell visuals from grid
    void UpdateCellVisuals();

    // Check if edge between two cells is fenced
    bool IsEdgeFencedBetweenCells(int32 X1, int32 Y1, int32 X2, int32 Y2);

    // Find shortest path from start to goal using BFS, returning path
    TArray<FCellPos> FindPathBFS(const FCellPos& Start, const FCellPos& Goal);

    // Get all human positions
    TArray<FCellPos> GetHumanPositions() const;

    // Get random empty cell
    FCellPos GetRandomEmptyCell() const;

    // Compute totals for HUD
    void ComputeTotals();
};
