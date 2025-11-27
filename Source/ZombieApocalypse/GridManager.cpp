#include "GridManager.h"
#include "Containers/Queue.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

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
        UE_LOG(LogTemp, Warning, TEXT("GridManager: Not Able to get mouse position"));
        return;
    }

    FHitResult Hit;
    bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);
    if (!bHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridManager: No HIT for the cursor trace!"));
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
    UpdatePreview(BuildPoints[BestIndex]); // Updates the previewmesh location AndOr Rotation
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
    if (!FencePreviewActor) return;
    FencePreviewActor->SetActorLocation(Point.WorldPosition);

    // Rotation depends on bool
    FRotator Rotation = Point.bIsHorizontal ? FRotator::ZeroRotator : FRotator(0.f, 90.f, 0.f);
    FencePreviewActor->SetActorRotation(Rotation);

    FencePreviewActor->SetActorHiddenInGame(false);
}

void AGridManager::TryPlaceFenceAtCurrentHover()
{
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
        FRotator Rot = Point.bIsHorizontal ? FRotator::ZeroRotator : FRotator(0.f, 90.f, 0.f);
        GetWorld()->SpawnActor<AActor>(FinalFenceClass, Point.WorldPosition, Rot);
    }

    // hide preview after building
    HidePreview();

    // Debug message
    UE_LOG(LogTemp, Log, TEXT("Fence placed! Index %d, %s"), Point.FenceIndex, Point.bIsHorizontal ? TEXT("Horizontal") : TEXT("Vertical"));
}

void AGridManager::GenerateBuildPoints()
{
    /* == based on Grid | Make BuildPoints == */
    const FVector BaseLoc = GetActorLocation();

    /// Vertical Points | Between Rows going -> |
    for (int32 X = 0; X < GridSize-1; X++) // 0 -> 8 // should be 9 | X
    {
        for (int32 Y = 0; Y < GridSize; Y++) // 0 -> 9 | should be 10 | Y
        {
            int32 Index = GetVerticalFenceIndex(X, Y);
            FVector Position = BaseLoc + FVector(
                Y * CellSize + (CellSize * 0.5),
                X * CellSize + (CellSize),
                0
            );
            
            // Add to array
            FBuildPoint& NewPoint = BuildPoints.Add_GetRef(FBuildPoint());
            NewPoint.WorldPosition = Position;
            NewPoint.FenceIndex = Index;
            NewPoint.bIsHorizontal = false;

            // Debug
            DrawDebugSphere(GetWorld(), Position, 15.f, 8, FColor::Green, false, 10.f);
            DrawDebugString(GetWorld(), Position + FVector(0, 0, 8), FString::Printf(TEXT("ID: %d (%d, %d)"), Index, X, Y), nullptr, FColor::Black, 10.f, false);
        }
    }

    /// Horizontal Points
    for (int32 Y = 0; Y < GridSize - 1; Y++) // 0 -> 8 // should be 9 | X
    {
        for (int32 X = 0; X < GridSize; X++) // 0 -> 9 | should be 10 | Y
        {
            int32 Index = GetHorizontalFenceIndex(X, Y);
            FVector Position = BaseLoc + FVector(
                Y * CellSize + (CellSize),
                X * CellSize + (CellSize * 0.5),
                0
            );

            FBuildPoint& NewPoint = BuildPoints.Add_GetRef(FBuildPoint());
            NewPoint.WorldPosition = Position;
            NewPoint.FenceIndex = Index;
            NewPoint.bIsHorizontal = true;

            // Debug
            DrawDebugSphere(GetWorld(), Position, 15.f, 8, FColor::Blue, false, 10.f);
            DrawDebugString(GetWorld(), Position + FVector(0, 0, 8), FString::Printf(TEXT("ID: %d (%d, %d)"), Index, X, Y), nullptr, FColor::Black, 10.f, false);

        }
    }
}

// Find Fence Index between Columns = Horizontal
int32 AGridManager::GetHorizontalFenceIndex(int32 X, int32 Y) const 
{
    // Row * Lenght + Col = math
    return Y * GridSize + X;
}

// Find Fence Index between Rows = Vertical
int32 AGridManager::GetVerticalFenceIndex(int32 X, int32 Y) const
{
    // Row * Lenght + Col = math
    return Y * (GridSize - 1) + X;
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
    if (X1 == X2) // Horizontal move 
    {
        int32 FenceY = FMath::Min(Y1, Y2);
        return HorizontalFence[GetHorizontalFenceIndex(X1, FenceY)];
    }
    else if (Y1 == Y2)
    {
        int32 FenceX = FMath::Min(X1, X2);
        return VerticalFence[GetVerticalFenceIndex(FenceX, Y1)];
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
