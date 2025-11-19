// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridManager.generated.h"


UENUM(BlueprintType)
enum class ECellState : uint8
{
    Empty   UMETA(DisplayName = "Empty"),
    Human   UMETA(DisplayName = "Human"),
    Zombie  UMETA(DisplayName = "Zombie")
};
 
UENUM(BlueprintType)
enum class EEdgeDirection : uint8
{
    Top,
    Bottom,
    Left,
    Right
};
 
USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Occupants")
    ECellState State = ECellState::Empty; // Anyone on the cell? - Might be redundant, remove later?

    UPROPERTY(BlueprintReadWrite, Category = "Occupants")
    bool bHasHuman = false; // For fast queries

    UPROPERTY(BlueprintReadWrite, Category = "Occupants")
    bool bAlreadyTarget = false; // use to make sure it does not get targeted by more zombies

    bool HasSusceptible() const { return bHasHuman; } // function to check if there is a human here
};
 
USTRUCT()
struct FGridNode
{
    GENERATED_BODY()
 
    int32 X = 0;
    int32 Y = 0;
 
    FGridNode() {}
    FGridNode(int32 InX, int32 InY) : X(InX), Y(InY) {}
 
    bool operator==(const FGridNode& Other) const { return X == Other.X && Y == Other.Y; }
    friend uint32 GetTypeHash(const FGridNode& Node)
    {
        return HashCombine(::GetTypeHash(Node.X), ::GetTypeHash(Node.Y));
    }
};



UCLASS()
class ZOMBIEAPOCALYPSE_API AGridManager : public AActor
{
    GENERATED_BODY()

public:

    static constexpr int32 GridSize = 10;
    
    AGridManager();

    // Cell info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Visuals")
    float CellSize = 100.f; // world units per cell

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShouldDebug = false;

    // Flattened 2D grid using TArray
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGridCell> Grid;
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> HorizontalFence; // size = GridSize * (GridSize + 1)
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> VerticalFence;   // size = (GridSize + 1) * GridSize


    // Index helpers for 2D access
    FORCEINLINE int32 GetGridIndex(int32 X, int32 Y) const { return X + Y * GridSize; }
    FORCEINLINE int32 GetHorizontalFenceIndex(int32 X, int32 Y) const { return X + Y * GridSize; }
    FORCEINLINE int32 GetVerticalFenceIndex(int32 X, int32 Y) const { return X * GridSize + Y; }

    
    bool IsValidCell(int32 X, int32 Y) const;
 
    void PlaceFence(int32 CellX, int32 CellY, EEdgeDirection Edge);
 
    bool IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const;
 
    bool CanMoveBetweenCells(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const;
 
    void GetNeighbors(const FGridNode& Node, TArray<FGridNode>& OutNeighbors) const;
 
    bool FindPath(const FGridNode& Start, const FGridNode& End, TArray<FGridNode>& OutPath) const;

    FVector GetCellCenterWorldPos(int32 X, int32 Y) const;
    FVector GetEdgeWorldPos(int32 EdgeX, int32 EdgeY, bool bIsHorizontal) const; //Snap point
    EEdgeDirection GetEdgeDirectionFromMouse(FVector WorldLoc) const; // Player targeting
};