#include "GridManager.h"
#include "Containers/CircularQueue.h"  // <- Changed to CircularQueue
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"  // For FMath::Min, etc.

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = true; // For debug draw

    Grid.SetNum(GridSize * GridSize);
    // Note: Initialization of NumSusceptible=1, NumZombies etc. now in SimulationController::BeginPlay

    HorizontalFence.Init(false, GridSize * (GridSize + 1));
    VerticalFence.Init(false, (GridSize + 1) * GridSize);
}

void AGridManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bShouldDebug) return;

    float Z = 10.f;
    // Draw horizontal fences
    for (int32 GridLineY = 0; GridLineY <= GridSize; ++GridLineY)
    {
        for (int32 X = 0; X < GridSize; ++X)
        {
            int32 Index = GetHorizontalFenceIndex(X, GridLineY);
            if (HorizontalFence[Index])
            {
                FVector Start = GetActorLocation() + FVector(X * CellSize, GridLineY * CellSize, Z);
                FVector End = Start + FVector(CellSize, 0, 0);
                DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 3.f);
            }
        }
    }
    // Draw vertical fences
    for (int32 GridLineX = 0; GridLineX <= GridSize; ++GridLineX)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            int32 Index = GetVerticalFenceIndex(GridLineX, Y);
            if (VerticalFence[Index])
            {
                FVector Start = GetActorLocation() + FVector(GridLineX * CellSize, Y * CellSize, Z);
                FVector End = Start + FVector(0, CellSize, 0);
                DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, -1.f, 0, 3.f);
            }
        }
    }

    // Draw occupants (stacked by type)
    for (int32 Y = 0; Y < GridSize; ++Y)
    {
        for (int32 X = 0; X < GridSize; ++X)
        {
            int Index = GetGridIndex(X, Y);
            FVector Center = GetCellCenterWorldPos(X, Y);
            const FGridCell& Cell = Grid[Index];

            if (Cell.NumSusceptible > 0)
                DrawDebugBox(GetWorld(), Center + FVector(0,0,5), FVector(CellSize*0.3f, CellSize*0.3f, 5.f), FQuat::Identity, FColor::Green, false, -1.f, 0, 1.f);
            if (Cell.NumBitten > 0)
                DrawDebugBox(GetWorld(), Center + FVector(0,0,15), FVector(CellSize*0.3f, CellSize*0.3f, 5.f), FQuat::Identity, FColor::Orange, false, -1.f, 0, 1.f);
            if (Cell.NumZombies > 0)
                DrawDebugBox(GetWorld(), Center + FVector(0,0,25), FVector(CellSize*0.3f, CellSize*0.3f, 5.f), FQuat::Identity, FColor::Red, false, -1.f, 0, 1.f);
        }
    }
}

bool AGridManager::IsValidCell(int32 X, int32 Y) const
{
    return X >= 0 && X < GridSize && Y >= 0 && Y < GridSize;
}

void AGridManager::PlaceFence(int32 CellX, int32 CellY, EEdgeDirection Edge)
{
    if (bShouldDebug)
        UE_LOG(LogTemp, Warning, TEXT("Placing fence on cell (%d,%d) -> %s"),
            CellX, CellY, *UEnum::GetValueAsString(Edge));

    if (!IsValidCell(CellX, CellY)) return;

    switch (Edge)
    {
    case EEdgeDirection::Top:
        HorizontalFence[GetHorizontalFenceIndex(CellX, CellY)] = true;     // Top edge of cell
        break;
    case EEdgeDirection::Bottom:
        HorizontalFence[GetHorizontalFenceIndex(CellX, CellY + 1)] = true; // Bottom edge
        break;
    case EEdgeDirection::Left:
        VerticalFence[GetVerticalFenceIndex(CellX, CellY)] = true;         // Left edge
        break;
    case EEdgeDirection::Right:
        VerticalFence[GetVerticalFenceIndex(CellX + 1, CellY)] = true;     // Right edge
        break;
    }
}

bool AGridManager::IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const
{
    if (X1 == X2)  // Vertical move (up/down, same X)
    {
        int32 FenceY = FMath::Min(Y1, Y2);
        return HorizontalFence[GetHorizontalFenceIndex(X1, FenceY + 1)];
    }
    else if (Y1 == Y2)  // Horizontal move (left/right, same Y)
    {
        int32 FenceX = FMath::Min(X1, X2);
        return VerticalFence[GetVerticalFenceIndex(FenceX + 1, Y1)];
    }
    return true;
}

bool AGridManager::CanMoveBetweenCells(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const
{
    return IsValidCell(ToX, ToY) && !IsEdgeBlockedByFence(FromX, FromY, ToX, ToY);
}

void AGridManager::GetNeighbors(const FGridNode& Node, TArray<FGridNode>& OutNeighbors) const
{
    static constexpr int32 DX[4] = { -1, 1, 0, 0 };
    static constexpr int32 DY[4] = { 0, 0, -1, 1 };

    OutNeighbors.Reset();
    for (int32 i = 0; i < 4; ++i)
    {
        int32 NX = Node.X + DX[i];
        int32 NY = Node.Y + DY[i];
        if (CanMoveBetweenCells(Node.X, Node.Y, NX, NY))
            OutNeighbors.Add(FGridNode(NX, NY));
    }
}

bool AGridManager::FindPath(const FGridNode& Start, const FGridNode& End, TArray<FGridNode>& OutPath) const
{
    if (bShouldDebug)
        UE_LOG(LogTemp, Warning, TEXT("FINDPATH: from (%d,%d) to (%d,%d)"),
            Start.X, Start.Y, End.X, End.Y);

    if (!IsValidCell(Start.X, Start.Y) || !IsValidCell(End.X, End.Y)) return false;

    TMap<FGridNode, FGridNode> CameFrom;
    TCircularQueue<FGridNode> Queue(GridSize * GridSize + 1);
    TSet<FGridNode> Visited;

    Queue.Enqueue(Start);
    Visited.Add(Start);
    CameFrom.Add(Start, Start);  // Sentinel

    while (!Queue.IsEmpty())
    {
        FGridNode Current;
        if (!Queue.Dequeue(Current)) continue;  // Safety, though shouldn't happen

        if (Current == End)
        {
            // Reconstruct path
            OutPath.Reset();
            FGridNode At = End;
            while (At != Start)
            {
                OutPath.Insert(At, 0);
                At = CameFrom[At];
            }
            OutPath.Insert(Start, 0);
            return true;
        }

        TArray<FGridNode> Neighbors;
        GetNeighbors(Current, Neighbors);
        for (const FGridNode& Neighbor : Neighbors)
        {
            if (!Visited.Contains(Neighbor))
            {
                Visited.Add(Neighbor);
                CameFrom.Add(Neighbor, Current);
                Queue.Enqueue(Neighbor);
            }
        }
    }
    return false;
}

FVector AGridManager::GetCellCenterWorldPos(int32 X, int32 Y) const
{
    return GetActorLocation() + FVector(X * CellSize + CellSize * 0.5f, Y * CellSize + CellSize * 0.5f, 0.f);
}

FVector AGridManager::GetEdgeWorldPos(int32 EdgeX, int32 EdgeY, bool bIsHorizontal) const
{
    FVector Base = GetActorLocation();
    if (bIsHorizontal)
        return Base + FVector(EdgeX * CellSize + CellSize * 0.5f, EdgeY * CellSize, 0.f);
    else
        return Base + FVector(EdgeX * CellSize, EdgeY * CellSize + CellSize * 0.5f, 0.f);
}

EEdgeDirection AGridManager::GetEdgeDirectionFromMouse(FVector WorldLoc) const
{
    FVector Local = WorldLoc - GetActorLocation();
    float Fx = Local.X / CellSize;
    float Fy = Local.Y / CellSize;
    int32 CellX = FMath::FloorToInt(Fx);
    int32 CellY = FMath::FloorToInt(Fy);
    float FracX = Fx - CellX;
    float FracY = Fy - CellY;

    // Distances to edges (normalized 0-1)
    float DistLeft   = FracX;
    float DistRight  = 1.f - FracX;
    float DistTop    = FracY;
    float DistBottom = 1.f - FracY;

    float MinDist = FMath::Min(FMath::Min(DistLeft, DistRight), FMath::Min(DistTop, DistBottom));  // Nested Min instead of Min4

    if (FMath::IsNearlyEqual(MinDist, DistLeft))   return EEdgeDirection::Left;
    if (FMath::IsNearlyEqual(MinDist, DistRight))  return EEdgeDirection::Right;
    if (FMath::IsNearlyEqual(MinDist, DistTop))    return EEdgeDirection::Top;
    return EEdgeDirection::Bottom;
}

int AGridManager::FindMinDistToHuman(const FGridNode& Start, FGridNode& OutTarget) const
{
    if (bShouldDebug)
        UE_LOG(LogTemp, Log, TEXT("FindMinDistToHuman from (%d,%d)"), Start.X, Start.Y);

    OutTarget = FGridNode(-1, -1);
    TCircularQueue<FGridNode> Queue(GridSize * GridSize + 1);
    TSet<FGridNode> Visited;
    Queue.Enqueue(Start);
    Visited.Add(Start);

    int Level = 0;
    const int MAX_DIST = 999;

    while (!Queue.IsEmpty())
    {
        int LevelSize = Queue.Count();  // ← FIXED: .Count() for TCircularQueue
        TArray<FGridNode> AtMinLevel;
        bool bFoundThisLevel = false;

        for (int i = 0; i < LevelSize; ++i)
        {
            FGridNode Current;
            if (!Queue.Dequeue(Current)) continue;

            int Index = GetGridIndex(Current.X, Current.Y);
            if (Grid[Index].HasHuman())
            {
                bFoundThisLevel = true;
                AtMinLevel.Add(Current);
            }

            TArray<FGridNode> Neighbors;
            GetNeighbors(Current, Neighbors);
            for (const FGridNode& N : Neighbors)
            {
                if (!Visited.Contains(N))
                {
                    Queue.Enqueue(N);
                    Visited.Add(N);
                }
            }
        }

        if (bFoundThisLevel && !AtMinLevel.IsEmpty())
        {
            int RandIdx = FMath::RandRange(0, AtMinLevel.Num() - 1);
            OutTarget = AtMinLevel[RandIdx];
            return Level;
        }

        Level++;
        if (Level > MAX_DIST) break;
    }
    return MAX_DIST;
}

TArray<TArray<FGridNode>> AGridManager::GetConnectedComponents() const
{
    TArray<TArray<FGridNode>> Components;
    TArray<bool> Visited;
    Visited.Init(false, Grid.Num());

    for (int32 Y = 0; Y < GridSize; ++Y)
    {
        for (int32 X = 0; X < GridSize; ++X)
        {
            int32 Idx = GetGridIndex(X, Y);
            if (!Visited[Idx])
            {
                TArray<FGridNode> Component;
                TCircularQueue<FGridNode> Queue(GridSize * GridSize + 1);
                FGridNode Node(X, Y);
                Queue.Enqueue(Node);
                Visited[Idx] = true;

                while (!Queue.IsEmpty())
                {
                    FGridNode Current;
                    if (!Queue.Dequeue(Current)) continue;

                    Component.Add(Current);

                    TArray<FGridNode> Neighbors;
                    GetNeighbors(Current, Neighbors);
                    for (const FGridNode& N : Neighbors)
                    {
                        int32 NIdx = GetGridIndex(N.X, N.Y);
                        if (!Visited[NIdx])
                        {
                            Visited[NIdx] = true;
                            Queue.Enqueue(N);
                        }
                    }
                }
                Components.Add(Component);
            }
        }
    }
    return Components;
}