#include "GridManager.h"
#include "Containers/Queue.h"

AGridManager::AGridManager()
{
    Grid.SetNum(GridSize * GridSize);
    for (int32 i = 0; i < Grid.Num(); ++i)
    {
        Grid[i].State = ECellState::Empty;
    }
 
    HorizontalFence.Init(false, GridSize * (GridSize + 1));
    VerticalFence.Init(false, (GridSize + 1) * GridSize);
}
 
bool AGridManager::IsValidCell(int32 X, int32 Y) const
{
    return X >= 0 && X < GridSize && Y >= 0 && Y < GridSize;
}
 
void AGridManager::PlaceFence(int32 CellX, int32 CellY, EEdgeDirection Edge)
{
    if (bShouldDebug) // Logs in console
        UE_LOG(LogTemp, Warning, TEXT("Placing fence on cell (%d,%d) -> %s"),
            CellX, CellY, *UEnum::GetValueAsString(Edge));


    switch (Edge)
    {
    case EEdgeDirection::Top:
        if (IsValidCell(CellX, CellY))
            HorizontalFence[GetHorizontalFenceIndex(CellX, CellY)] = true; // Top edge of THIS cell
        break;
    case EEdgeDirection::Bottom:
        if (IsValidCell(CellX, CellY) && CellY < GridSize-1)
            HorizontalFence[GetHorizontalFenceIndex(CellX, CellY - 1)] = true; // Bottom = Top of cell below
        break;
    case EEdgeDirection::Left:
        if (IsValidCell(CellX, CellY) && CellX > 0)
            VerticalFence[GetVerticalFenceIndex(CellX, CellY)] = true; // Left edge of THIS cell
        break;
    case EEdgeDirection::Right:
        if (IsValidCell(CellX, CellY) && CellX < GridSize-1)
            VerticalFence[GetVerticalFenceIndex(CellX, CellY)] = true; // Right = Left of cell right
        break;
    }
}
 
bool AGridManager::IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const
{
    if (!IsValidCell(X1, Y1) || !IsValidCell(X2, Y2))
        return true;
 
    if (X1 == X2)
    {
        int32 MinY = FMath::Min(Y1, Y2);
        return HorizontalFence[GetHorizontalFenceIndex(X1, MinY)];
    }
    else if (Y1 == Y2)
    {
        int32 MinX = FMath::Min(X1, X2);
        return VerticalFence[GetVerticalFenceIndex(MinX, Y1)];
    }
    return true;
}

bool AGridManager::CanMoveBetweenCells(int32 FromX, int32 FromY, int32 ToX, int32 ToY) const
{
    return IsValidCell(ToX, ToY)
        && !IsEdgeBlockedByFence(FromX, FromY, ToX, ToY);
}

void AGridManager::GetNeighbors(const FGridNode& Node, TArray<FGridNode>& OutNeighbors) const
{
    static const int32 Dx[4] = { -1, 1, 0, 0 };
    static const int32 Dy[4] = { 0, 0, -1, 1 };
 
    for (int32 i = 0; i < 4; ++i)
    {
        int32 Nx = Node.X + Dx[i];
        int32 Ny = Node.Y + Dy[i];
        if (CanMoveBetweenCells(Node.X, Node.Y, Nx, Ny))
            OutNeighbors.Add(FGridNode(Nx, Ny));
    }
}


bool AGridManager::FindPath(const FGridNode& Start, const FGridNode& End, TArray<FGridNode>& OutPath) const
{
    UE_LOG(LogTemp, Warning, TEXT("FINDPATH: from (%d,%d) to (%d,%d)"),
        Start.X, Start.Y, End.X, End.Y);

    if (!IsValidCell(Start.X, Start.Y) || !IsValidCell(End.X, End.Y))
        return false;
 
    TMap<FGridNode, FGridNode> CameFrom;
    TQueue<FGridNode> Queue;
    TSet<FGridNode> Visited;
 
    Queue.Enqueue(Start);
    Visited.Add(Start);
 
    while (!Queue.IsEmpty())
    {
        FGridNode Current;
        Queue.Dequeue(Current);
 
        if (Current == End)
        {
            OutPath.Empty();
 
            FGridNode Node = End;
            while (!(Node == Start))
            {
                OutPath.Insert(Node, 0);
                Node = CameFrom[Node];
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
                Queue.Enqueue(Neighbor);
                Visited.Add(Neighbor);
                CameFrom.Add(Neighbor, Current);
            }
        }
    }
    return false;
}

FVector AGridManager::GetCellCenterWorldPos(int32 X, int32 Y) const
{
    return GetActorLocation() + FVector(X * CellSize + CellSize*0.5, Y * CellSize + CellSize*0.5f, 0.f);
}

FVector AGridManager::GetEdgeWorldPos(int32 EdgeX, int32 EdgeY, bool bIsHorizontal) const
{
    FVector Base = GetActorLocation();
    if (bIsHorizontal)
    {
        return Base + FVector(EdgeX * CellSize + CellSize * 0.5f, EdgeY * CellSize, 0.f);
    }
    else
    {
        return Base + FVector(EdgeX * CellSize, EdgeY * CellSize + CellSize * 0.5f, 0.f);
    }
}

EEdgeDirection AGridManager::GetEdgeDirectionFromMouse(FVector WorldLoc) const
{
    FVector Local = WorldLoc - GetActorLocation();
    int32 SnapX = FMath::RoundToInt(Local.X / CellSize);
    int32 SnapY = FMath::RoundToInt(Local.Y / CellSize);

    // Check distance to 4 possible edges, return closest
    float MinDist = MAX_FLT;
    EEdgeDirection BestDir = EEdgeDirection::Top;


    // Implement dist to each edge type
    return BestDir;

}
