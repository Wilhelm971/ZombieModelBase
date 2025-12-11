#include "GridManager.h"
#include "Containers/Queue.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    CurrentCoins = 0;

    Grid.SetNum(GridSize * GridSize);
    for (int32 i = 0; i < Grid.Num(); ++i)
    {
        Grid[i].State = ECellState::Empty;
    }

    HorizontalFence.Init(false, GridSize * (GridSize - 1)); // Between Rows        ---
    VerticalFence.Init(false, (GridSize - 1) * GridSize);   // Between Columns      |
}

void AGridManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //UE_LOG(LogTemp, Warning, TEXT("=== Build Mode Tick Activated ==="));

    if (!bBuildModeActive || BuildPoints.Num() == 0) return;
    

    APlayerController* PC = GetWorld()->GetFirstPlayerController(); // TEMP! Gotta change, gonna be looking each tick...
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridManager: No Player Controller!"));
        return;
    }

    FVector2D MousePos; // Get mouse position
    if (!PC->GetMousePosition(MousePos.X, MousePos.Y))
    {
        //UE_LOG(LogTemp, Warning, TEXT("GridManager: Not able to get mouse position"));
        return;
    }

    FHitResult Hit;
    bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
    if (!bHit)
    {
        //UE_LOG(LogTemp, Warning, TEXT("GridManager: No HIT for the cursor trace!"));
        HidePreview();
        return;
    }

    // Find closest BuildPoint
    int32 BestIndex = FindNearestBuildPoint(Hit.ImpactPoint);

    if (BestIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridManager: No BuildPoint close enough!"));
        HidePreview();
        return;
    }

    //UE_LOG(LogTemp, Warning, TEXT("GridManager: |---END---|"));
    CurrentHoveredPointIndex = BestIndex;

    if (CurrentCoins >= BuildCost)
    {
        UpdatePreview(BuildPoints[BestIndex]); // Updates the previewmesh location AndOr Rotation
    }
}

void AGridManager::EnterBuildMode()
{
    // Check if already active else set active
    if (bBuildModeActive) return;   
    bBuildModeActive = true;
    CurrentHoveredPointIndex = INDEX_NONE;

    // check if there is a preview
    if (!FencePreviewClass)         
    {
        UE_LOG(LogTemp, Warning, TEXT("GridManager: FencePreviewClass not set!"));
        return;
    }

    // Spawn preview actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FencePreviewActor = GetWorld()->SpawnActor<AActor>(FencePreviewClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    // Enable tick for live preview
    SetActorTickEnabled(true);
}

void AGridManager::ExitBuildMode()
{
    if (!bBuildModeActive) return;
    bBuildModeActive = false;
    CurrentHoveredPointIndex = INDEX_NONE;

    // If preview then delete
    if (FencePreviewActor)
    {
        FencePreviewActor->Destroy();
        FencePreviewActor = nullptr;
    }

    SetActorTickEnabled(false); // turn off and save perfomance
}

int32 AGridManager::FindNearestBuildPoint(const FVector& HitLocation)
{
    float ClosestDistSq = BIG_NUMBER;
    int32 BestIndex = INDEX_NONE;

    for (int32 i = 0; i < BuildPoints.Num(); ++i)
    {
        const FBuildPoint& Point = BuildPoints[i];
        float DistSq = (HitLocation - Point.WorldPosition).SizeSquared();
        if (DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            BestIndex = i;
        }
    }

    // Distance tolerance
    if (ClosestDistSq > FMath::Square(120.f))
    {
        return INDEX_NONE;
    }

    return BestIndex; // Return closest BuildPoint
}

void AGridManager::HidePreview()
{
    if (FencePreviewActor)
        FencePreviewActor->SetActorHiddenInGame(true);
    CurrentHoveredPointIndex = INDEX_NONE;
}

void AGridManager::UpdatePreview(const FBuildPoint& Point)
{
    if (Point.bIsUsed)
    {
        // Hide if something already buildt on location
        FencePreviewActor->SetActorHiddenInGame(true);
        return;
    }

    if (!FencePreviewActor) return;
    FencePreviewActor->SetActorLocation(Point.WorldPosition);

    // Rotation depends on bool
    FRotator Rotation = Point.bIsHorizontal ? FRotator(0.f, 90.f, 0.f) : FRotator::ZeroRotator;
    FencePreviewActor->SetActorRotation(Rotation);

    FencePreviewActor->SetActorHiddenInGame(false);
}

void AGridManager::TryPlaceFenceAtCurrentHover()
{
    if (CurrentCoins < BuildCost) return;
    
    if (CurrentHoveredPointIndex == INDEX_NONE) return;
    if (BuildPoints[CurrentHoveredPointIndex].bIsUsed) return;

    const FBuildPoint& Point = BuildPoints[CurrentHoveredPointIndex];

    // Mark as used
    BuildPoints[CurrentHoveredPointIndex].bIsUsed = true;

    // Actually place the fence in the correct array
    if (Point.bIsHorizontal)
        HorizontalFence[Point.FenceIndex] = true;
    else
        VerticalFence[Point.FenceIndex] = true;

    // Spawn real fence
    if (FinalFenceClass)
    {
        FRotator Rot = Point.bIsHorizontal ? FRotator(0.f, 90.f, 0.f) : FRotator::ZeroRotator;
        GetWorld()->SpawnActor<AActor>(FinalFenceClass, Point.WorldPosition, Rot);
        CurrentCoins -= BuildCost;
    }

    // hide preview after building
    HidePreview();

    // Debug message
    //UE_LOG(LogTemp, Log, TEXT("Fence placed! Index %d, %s"), Point.FenceIndex, Point.bIsHorizontal ? TEXT("Horizontal") : TEXT("Vertical"));
}

void AGridManager::GenerateBuildPoints()
{
    /* == based on Grid | Make BuildPoints == */
    const FVector BaseLoc = GetActorLocation();

    /// Vertical Points | Between Columns going -> | (blocks X)
    for (int32 ColumnGap = 0; ColumnGap < GridSize -1; ColumnGap++) // 0 -> 8 // should be 9 | X
    {
        for (int32 Row = 0; Row < GridSize; Row++) // 0 -> 9 | should be 10 | Y
        {
            int32 Index = GetVerticalFenceIndex(ColumnGap, Row);
            FVector Position = BaseLoc + FVector(
                (ColumnGap + 1) * CellSize,
                Row * CellSize + (CellSize * 0.5f),
                0
            );
            
            // Add to array
            FBuildPoint& NewPoint = BuildPoints.Add_GetRef(FBuildPoint());
            NewPoint.WorldPosition = Position;
            NewPoint.FenceIndex = Index;
            NewPoint.bIsHorizontal = false;

            // Debug
            //DrawDebugSphere(GetWorld(), Position, 15.f, 8, FColor::Green, false, 60.f);
            //DrawDebugString(GetWorld(), Position + FVector(0, 0, 8), FString::Printf(TEXT("ID: %d (%d, %d)"), Index, ColumnGap, Row), nullptr, FColor::Black, 60.f, false);
        }
    }

    /// Horizontal Points
    for (int32 RowGap = 0; RowGap < GridSize - 1; RowGap++) // 0 -> 8 // should be 9 | X
    {
        for (int32 Column = 0; Column < GridSize; Column++) // 0 -> 9 | should be 10 | Y
        {
            int32 Index = GetHorizontalFenceIndex(Column, RowGap);
            FVector Position = BaseLoc + FVector(
                Column * CellSize + (CellSize * 0.5),
                (RowGap + 1) * CellSize,
                0
            );

            FBuildPoint& NewPoint = BuildPoints.Add_GetRef(FBuildPoint());
            NewPoint.WorldPosition = Position;
            NewPoint.FenceIndex = Index;
            NewPoint.bIsHorizontal = true;

            // Debug
            //DrawDebugSphere(GetWorld(), Position, 15.f, 8, FColor::Blue, false, 60.f);
            //DrawDebugString(GetWorld(), Position + FVector(0, 0, 8), FString::Printf(TEXT("ID: %d (%d, %d)"), Index, Column, RowGap), nullptr, FColor::Black, 60.f, false);
        }
    }
}

// Find Fence Index between Columns = Horizontal
int32 AGridManager::GetHorizontalFenceIndex(int32 Column, int32 RowGap) const 
{
    // Row * Lenght + Col = math
    int32 ReturnValue = RowGap * GridSize + Column;
    if (ReturnValue > HorizontalFence.Num()-1)
    {
        return 0; // crash safe
    }
    return ReturnValue; // return Y * GridSize + X;
}

// Find Fence Index between Rows = Vertical
int32 AGridManager::GetVerticalFenceIndex(int32 X, int32 Y) const
{
    // Row * Lenght + Col = math
    int32 ReturnValue =  Y* (GridSize - 1) + X;
    if (ReturnValue > VerticalFence.Num()-1)
    {
        return 0; // crash safe
    }
    return ReturnValue; //return Y * (GridSize - 1) + X;
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

    if (!IsValidCell(CellX, CellY)) return;

    switch (Edge)
    {
    case EEdgeDirection::Top:
        HorizontalFence[GetHorizontalFenceIndex(CellX, CellY)] = true; // Top edge = line Y
        break;

    case EEdgeDirection::Bottom:
        HorizontalFence[GetHorizontalFenceIndex(CellX, CellY + 1)] = true; // Bottom edge = line Y+1
        break;

    case EEdgeDirection::Left:
        VerticalFence[GetVerticalFenceIndex(CellX, CellY)] = true; // Left edge = line X
        break;

    case EEdgeDirection::Right:
        VerticalFence[GetVerticalFenceIndex(CellX +1, CellY)] = true; // Right edge = line X+1
        break;

    }
}
 
bool AGridManager::IsEdgeBlockedByFence(int32 X1, int32 Y1, int32 X2, int32 Y2) const
{
    const int32 FenceIndex = GetFenceIndexBetweenCells(X1, Y1, X2, Y2);
    if (FenceIndex == INDEX_NONE)
        return true; // Not adjacent -> block (should never happen tho)

    //UE_LOG(LogTemp, Warning, TEXT("FENCE INDEX: %d"), (int32)FenceIndex);
    //UE_LOG(LogTemp, Warning, TEXT("X1: %d, Y1: %d, X2: %d, Y2: %d"), (int32)X1, (int32)Y1, (int32)X2, (int32)Y2);

    if (X1 == X2)
    {
        // Moving vertically (up/down) -> blocked by Horiztonal fence
        return HorizontalFence[FenceIndex];
    }
    else
    {
        // Moving horizontally (left/right) -> blocked by Vertical fence
        return VerticalFence[FenceIndex];
    }
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
    //UE_LOG(LogTemp, Warning, TEXT("FINDPATH: from (%d,%d) to (%d,%d)"),
    //    Start.X, Start.Y, End.X, End.Y);

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

void AGridManager::DebugPathfind(FVector2D Start, FVector2D Goal)
{
    FGridNode StartNode(Start.X, Start.Y);
    FGridNode GoalNode(Goal.X, Goal.Y);

    TArray<FGridNode> Path;
    bool bPathFound = FindPath(StartNode, GoalNode, Path);

    FString Result = bPathFound ?
        FString::Printf(TEXT("PATH FOUND! Length %d"), Path.Num()) : TEXT("NO PATH! (bug or blocked by fences");

    //UE_LOG(LogTemp, Warning, TEXT("DebugPathfind (%d, %d) -> (%d, %d): %s"),
    //    (int32)Start.X, (int32)Start.Y, (int32)Goal.X, (int32)Goal.Y, *Result);

    // Draw fences based on what the pathfinding sees
    const FVector Offset = FVector(0, 0, 5.f);

    /// Iterate over every possible neighbor pair in the 10x10 grid
    for (int32 X = 0; X < GridSize; ++X)
    {
        for (int32 Y = 0; Y < GridSize; ++Y)
        {
            FVector Center = GetCellCenterWorldPos(X, Y) + Offset;

            // Check RIGHT neighbor (X+1, Y)
            if (X < GridSize - 1)
            {
                bool bBlocked = !CanMoveBetweenCells(X, Y, X + 1, Y);
                if (bBlocked)
                {
                    FVector RightCenter = GetCellCenterWorldPos(X + 1, Y) + Offset;
                    FVector Start = FVector(Center.X + CellSize * 0.5f, Center.Y, Center.Z);
                    FVector End = FVector(Center.X + CellSize * 0.5f, Center.Y, Center.Z);
                    DrawDebugLine(GetWorld(), Start + FVector(0, -CellSize * 0.4f, 0),
                        Start + FVector(0, CellSize * 0.4f, 0),
                        FColor::Red, false, 5.f, 0, 5);
                }
            }

            // Check DOWN neighbor (X, Y+1)
            if (Y < GridSize - 1)
            {
                bool bBlocked = !CanMoveBetweenCells(X, Y, X, Y + 1);
                if (bBlocked)
                {
                    FVector DownCenter = GetCellCenterWorldPos(X, Y + 1) + Offset;
                    FVector Start = FVector(Center.X, Center.Y + CellSize * 0.5f, Center.Z);
                    FVector End = FVector(Center.X, Center.Y + CellSize * 0.5f, Center.Z);
                    DrawDebugLine(GetWorld(), Start + FVector(-CellSize * 0.4f, 0, 0),
                        Start + FVector(CellSize * 0.4f, 0, 0),
                        FColor::Red, false, 5.f, 0, 5);
                }
            }

        }
    }


    // Draw start/Goal regardless
    DrawDebugSphere(GetWorld(), GetCellCenterWorldPos(Start.X, Start.Y), 35.f, 16, FColor::Yellow, false, 5.f);
    DrawDebugSphere(GetWorld(), GetCellCenterWorldPos(Goal.X, Goal.Y), 35.f, 16, FColor::Orange, false, 5.f);

    if (bPathFound && Path.Num() > 0)
    {
        // Green spheres on EVERY path cell
        for (const FGridNode& Node : Path)
        {
            DrawDebugSphere(GetWorld(), GetCellCenterWorldPos(Node.X, Node.Y), 28.f, 16, FColor::Green, false, 5.f);
        }

        // Thick green lines connecting pathe
        for (int32 i = 1; i < Path.Num(); ++i)
        {
            FVector P1 = GetCellCenterWorldPos(Path[i - 1].X, Path[i - 1].Y);
            FVector P2 = GetCellCenterWorldPos(Path[i].X, Path[i].Y);
            DrawDebugLine(GetWorld(), P1, P2, FColor::Green, false, 5.f, 0, 4.f);
        }
    }
    else
    {
        // Draw red X on goal if bloced (fun visual)
        FVector GoalPos = GetCellCenterWorldPos(Goal.X, Goal.Y);
        DrawDebugLine(GetWorld(), GoalPos + FVector(-20, -20, 0), GoalPos + FVector(20, 20, 0), FColor::Red, false, 30.f, 0, 3.f);
        DrawDebugLine(GetWorld(), GoalPos + FVector(20, -20, 0), GoalPos + FVector(-20, 20, 0), FColor::Red, false, 30.f, 0, 3.f);
    }
}

int32 AGridManager::GetFenceIndexBetweenCells(int32 CellX1, int32 CellY1, int32 CellX2, int32 CellY2) const
{
    if (!IsValidCell(CellX1, CellY1) || !IsValidCell(CellX2, CellY2))
        return INDEX_NONE;

    const int32 DX = CellX2 - CellX1;
    const int32 DY = CellY2 - CellY1;

    // Must be exactly ONE step in cardinal direction
    if (FMath::Abs(DX) + FMath::Abs(DY) != 1)
        return INDEX_NONE;
    
    if (DX == 0) // Vertical movement -> horizontal fence between rows
    {
        const int32 FenceRow = FMath::Min(CellY1, CellY2); // Fence is above the lower cell
        const int32 Col = CellX1;
        return GetHorizontalFenceIndex(Col, FenceRow);
    }
    else // DY == 0 -> Horizontal movement -> Vertical fence between columns
    {
        const int32 FenceCol = FMath::Min(CellX1, CellX2); // Fence is left of the right cell
        const int32 Row = CellY1;
        return GetVerticalFenceIndex(FenceCol, Row);
    }
}
