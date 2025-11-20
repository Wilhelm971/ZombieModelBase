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
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECellState State = ECellState::Empty;
 
    bool IsWalkable() const
    {
        return State != ECellState::Zombie; // Example: Zombies block walkability, adjust as needed
    }
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
    FORCEINLINE int32 GetVerticalFenceIndex(int32 X, int32 Y) const { return X + Y * (GridSize + 1); }

    
    bool IsValidCell(int32 X, int32 Y) const;
 
    
    void PlaceFence(int32 CellX, int32 CellY, EEdgeDirection Edge);
 

    bool IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const;
 

    bool CanMoveBetweenCells(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const;
 
    void GetNeighbors(const FGridNode& Node, TArray<FGridNode>& OutNeighbors) const;
 
    bool FindPath(const FGridNode& Start, const FGridNode& End, TArray<FGridNode>& OutPath) const;
};