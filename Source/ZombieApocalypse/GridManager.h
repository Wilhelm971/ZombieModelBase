// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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

// structure for BuildPoints
USTRUCT()
struct FBuildPoint 
{
    GENERATED_BODY()

    FVector WorldPosition;
    int32 FenceIndex = INDEX_NONE;
    bool bIsHorizontal = false; // changed later
    bool bIsUsed = false;
};

UCLASS()
class ZOMBIEAPOCALYPSE_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();
    virtual void Tick(float DeltaTime) override;

    // BuildMode logic:
    UFUNCTION()
    void EnterBuildMode();
    void ExitBuildMode();
    int32 FindNearestBuildPoint(const FVector& WorldLocation);
    void HidePreview();
    void UpdatePreview(const FBuildPoint& Point);
    void TryPlaceFenceAtCurrentHover();
    void GenerateBuildPoints();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
    TSubclassOf<AActor> FencePreviewClass; // Assign a translucent fence BP in editor

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Building")
    TSubclassOf<AActor> FinalFenceClass; // Real Fence beeing placed, Assign in editor


    static constexpr int32 GridSize = 10;

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
    FORCEINLINE int32 GetHorizontalFenceIndex(int32 Col, int32 Row) const;
    FORCEINLINE int32 GetVerticalFenceIndex(int32 GridLineX, int32 CellY) const;
    
    bool IsValidCell(int32 X, int32 Y) const;
 
    void PlaceFence(int32 CellX, int32 CellY, EEdgeDirection Edge);
 
    bool IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const;
 
    bool CanMoveBetweenCells(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const;
 
    void GetNeighbors(const FGridNode& Node, TArray<FGridNode>& OutNeighbors) const;
 
    bool FindPath(const FGridNode& Start, const FGridNode& End, TArray<FGridNode>& OutPath) const;

    FVector GetCellCenterWorldPos(int32 X, int32 Y) const;
    FVector GetEdgeWorldPos(int32 EdgeX, int32 EdgeY, bool bIsHorizontal) const; //Snap point
    EEdgeDirection GetEdgeDirectionFromMouse(FVector WorldLoc) const; // Player targeting

private:

    // BuildModeState
    bool bBuildModeActive = false;

    // Preview logic:
    UPROPERTY()
    AActor* FencePreviewActor = nullptr;

    // Currently hovered build point (-1 = none)
    int32 CurrentHoveredPointIndex = INDEX_NONE;

    // Build points array
    TArray<FBuildPoint> BuildPoints;
};