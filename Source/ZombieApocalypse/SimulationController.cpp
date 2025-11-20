// Copyright University of Inland Norway

#include "SimulationController.h"

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = false; // Turn-based, no need for Tick

    // Directions
    Directions = {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0}
    };
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();

    // Checking if the DataTable is assigned (kept if needed)
    if (!PopulationDensityEffectTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable is not assigned!"));
    }
    else
    {
        ReadDataFromTableToVectors();
    }

    // Calculate max edges
    MaxEdges = (GridSizeY + 1) * GridSizeX + GridSizeY * (GridSizeX + 1);
    FencedEdges.Init(false, MaxEdges);

    // Spawn the grid
    SpawnGrid();

    // Initialize grid occupants
    GridOccupants.SetNum(GridSizeY);
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        GridOccupants[Y].Init(ECellType::Empty, GridSizeX);
    }

    // Place initial humans and zombie randomly on empty cells
    TArray<FCellPos> AllCells;
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            AllCells.Add({X, Y});
        }
    }

    // Shuffle for random placement
    for (int32 i = AllCells.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Swap(AllCells[i], AllCells[j]);
    }

    // Place humans
    for (int32 i = 0; i < InitialHumans && i < AllCells.Num(); ++i)
    {
        FCellPos Pos = AllCells[i];
        GridOccupants[Pos.Y][Pos.X] = ECellType::Human;
    }

    // Place zombie (overwrite if on human, but since extra cells, place on empty)
    FCellPos ZombiePos = GetRandomEmptyCell();
    GridOccupants[ZombiePos.Y][ZombiePos.X] = ECellType::Zombie;

    // Initial totals and visuals
    ComputeTotals();
    UpdateCellVisuals();
}

void ASimulationController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // No auto-ticking; player-driven
}

void ASimulationController::ReadDataFromTableToVectors()
{
    if (bShouldDebug)
        UE_LOG(LogTemp, Log, TEXT("Read Data From Table To Vectors"));

    TArray<FName> RowNames = PopulationDensityEffectTable->GetRowNames();

    for (int i = 0; i < RowNames.Num(); i++)
    {
        if (bShouldDebug)
            UE_LOG(LogTemp, Log, TEXT("Reading table row index: %d"), i);

        FPopulationDensityEffect* RowData = PopulationDensityEffectTable->FindRow<FPopulationDensityEffect>(RowNames[i], TEXT(""));
        if (RowData)
        {
            graphPts.Add(std::make_pair(RowData->PopulationDensity, RowData->NormalPopulationDensity));

            if (bShouldDebug)
            {
                auto LastPair = graphPts.Last();
                UE_LOG(LogTemp, Warning, TEXT("Reading table row: %d, pair: (%f, %f)"), i, LastPair.first, LastPair.second);
            }
        }
    }
}

void ASimulationController::AdvanceOneDay()
{
    CurrentDay++;

    // Collect all zombie positions
    TArray<FCellPos> ZombiePositions;
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            if (GridOccupants[Y][X] == ECellType::Zombie)
            {
                ZombiePositions.Add({X, Y});
            }
        }
    }

    // Movement phase: Each zombie moves toward closest human
    TArray<FCellPos> HumanPositions = GetHumanPositions();
    for (const FCellPos& ZombiePos : ZombiePositions)
    {
        if (HumanPositions.IsEmpty()) continue;

        // Find closest human (Manhattan distance for approximation)
        FCellPos ClosestHuman;
        float MinDist = MAX_flt;
        for (const FCellPos& HumanPos : HumanPositions)
        {
            float Dist = FMath::Abs(ZombiePos.X - HumanPos.X) + FMath::Abs(ZombiePos.Y - HumanPos.Y);
            if (Dist < MinDist)
            {
                MinDist = Dist;
                ClosestHuman = HumanPos;
            }
        }

        // Get path to closest human
        TArray<FCellPos> Path = FindPathBFS(ZombiePos, ClosestHuman);
        if (Path.Num() > 1)  // At least one step
        {
            FCellPos NextPos = Path[1];  // First step
            if (GridOccupants[NextPos.Y][NextPos.X] == ECellType::Empty)
            {
                // Move
                GridOccupants[ZombiePos.Y][ZombiePos.X] = ECellType::Empty;
                GridOccupants[NextPos.Y][NextPos.X] = ECellType::Zombie;
            }
        }
    }

    // Biting phase: Zombies bite adjacent or same cell humans
    ZombiePositions.Empty();
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            if (GridOccupants[Y][X] == ECellType::Zombie)
            {
                ZombiePositions.Add({X, Y});
            }
        }
    }

    for (const FCellPos& ZombiePos : ZombiePositions)
    {
        // Check same cell (if moved onto human, but since move only to empty, check adjacent
        for (const FNeighbor& Dir : Directions)
        {
            int32 NX = ZombiePos.X + Dir.DX;
            int32 NY = ZombiePos.Y + Dir.DY;
            if (NX >= 0 && NX < GridSizeX && NY >= 0 && NY < GridSizeY)
            {
                if (GridOccupants[NY][NX] == ECellType::Human && !IsEdgeFencedBetweenCells(ZombiePos.X, ZombiePos.Y, NX, NY))
                {
                    // Bite
                    GridOccupants[NY][NX] = ECellType::Empty;
                    FBitten NewBitten;
                    NewBitten.RemainingDays = DaysToBecomeZombie;
                    Bitten.Add(NewBitten);
                    break;  // One bite per zombie per turn
                }
            }
        }
    }

    // Age bitten
    for (FBitten& Bit : Bitten)
    {
        Bit.RemainingDays -= 1;
    }

    // Transform to zombie on empty cell
    for (int32 i = Bitten.Num() - 1; i >= 0; --i)
    {
        if (Bitten[i].RemainingDays <= 0)
        {
            FCellPos EmptyPos = GetRandomEmptyCell();
            if (EmptyPos.X != -1)
            {
                GridOccupants[EmptyPos.Y][EmptyPos.X] = ECellType::Zombie;
            }
            Bitten.RemoveAt(i);
        }
    }

    // Totals and visuals
    ComputeTotals();
    UpdateCellVisuals();

    if (bShouldDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("Day %d: H=%d, B=%d, Z=%d"), CurrentDay, CurrentHumans, Bitten.Num(), CurrentZombies);
    }

    OnDayAdvanced.Broadcast(CurrentDay);

    if (CurrentHumans <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Humanity lost!"));
    }
    else if (CurrentZombies <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Outbreak contained!"));
    }
}

void ASimulationController::PlaceFence(int32 EdgeID)
{
    if (EdgeID >= 0 && EdgeID < MaxEdges)
    {
        FencedEdges[EdgeID] = true;
        if (bShouldDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("Fence placed on edge %d"), EdgeID);
        }
    }
}

void ASimulationController::SpawnGrid()
{
    if (!EdgeActorClass || !CellActorClass) 
    {
        UE_LOG(LogTemp, Error, TEXT("EdgeActorClass or CellActorClass not assigned!"));
        return;
    }

    int32 EdgeCounter = 0;
    FVector BaseLocation = GetActorLocation();
    CellActors.Reserve(GridSizeX * GridSizeY);

    // Spawn horizontal edges
    for (int32 Y = 0; Y <= GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            FVector Location = BaseLocation + FVector(X * CellSpacing, Y * CellSpacing, 0.f);
            FRotator Rotation(0.f, 0.f, 0.f);

            AEdgeActor* NewEdge = GetWorld()->SpawnActor<AEdgeActor>(EdgeActorClass, Location, Rotation);
            if (NewEdge)
            {
                NewEdge->EdgeID = EdgeCounter++;
                NewEdge->SetActorScale3D(FVector(CellSpacing / 100.f, 0.1f, 0.1f));
            }
        }
    }

    // Spawn vertical edges
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X <= GridSizeX; ++X)
        {
            FVector Location = BaseLocation + FVector(X * CellSpacing, Y * CellSpacing, 0.f);
            FRotator Rotation(0.f, 90.f, 0.f);

            AEdgeActor* NewEdge = GetWorld()->SpawnActor<AEdgeActor>(EdgeActorClass, Location, Rotation);
            if (NewEdge)
            {
                NewEdge->EdgeID = EdgeCounter++;
                NewEdge->SetActorScale3D(FVector(0.1f, CellSpacing / 100.f, 0.1f));
            }
        }
    }

    // Spawn cells at centers
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            FVector Location = BaseLocation + FVector((X + 0.5f) * CellSpacing, (Y + 0.5f) * CellSpacing, 0.f);
            FRotator Rotation(0.f, 0.f, 0.f);

            ACellActor* NewCell = GetWorld()->SpawnActor<ACellActor>(CellActorClass, Location, Rotation);
            if (NewCell)
            {
                NewCell->CellIndex = Y * GridSizeX + X;
                CellActors.Add(NewCell);
            }
        }
    }
}

void ASimulationController::ComputeTotals()
{
    CurrentHumans = 0;
    CurrentZombies = 0;

    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            if (GridOccupants[Y][X] == ECellType::Human)
            {
                CurrentHumans++;
            }
            else if (GridOccupants[Y][X] == ECellType::Zombie)
            {
                CurrentZombies++;
            }
        }
    }
}

void ASimulationController::UpdateCellVisuals()
{
    if (CellActors.IsEmpty()) return;

    for (int32 Index = 0; Index < CellActors.Num(); ++Index)
    {
        ACellActor* Cell = CellActors[Index];
        if (!Cell) continue;

        int32 X = Index % GridSizeX;
        int32 Y = Index / GridSizeX;

        float LocalSus = (GridOccupants[Y][X] == ECellType::Human) ? 1.f : 0.f;
        float LocalBitten = 0.f;  // Bitten off-map
        float LocalZom = (GridOccupants[Y][X] == ECellType::Zombie) ? 1.f : 0.f;

        Cell->SetDominantPopulation(LocalSus, LocalBitten, LocalZom);
    }
}

bool ASimulationController::IsEdgeFencedBetweenCells(int32 X1, int32 Y1, int32 X2, int32 Y2)
{
    int32 DX = X2 - X1;
    int32 DY = Y2 - Y1;
    int32 EdgeID = -1;

    if (DY != 0)  // Horizontal edge
    {
        int32 EdgeY = FMath::Min(Y1, Y2);
        int32 EdgeX = X1;
        EdgeID = EdgeY * GridSizeX + EdgeX;
    }
    else if (DX != 0)  // Vertical edge
    {
        int32 EdgeX = FMath::Min(X1, X2);
        int32 EdgeY = Y1;
        int32 HorizontalsCount = (GridSizeY + 1) * GridSizeX;
        EdgeID = HorizontalsCount + EdgeY * (GridSizeX + 1) + EdgeX;
    }

    return (EdgeID >= 0 && EdgeID < MaxEdges && FencedEdges[EdgeID]);
}

TArray<FCellPos> ASimulationController::FindPathBFS(const FCellPos& Start, const FCellPos& Goal)
{
    TQueue<FCellPos> Queue;
    Queue.Enqueue(Start);

    TMap<FCellPos, FCellPos> CameFrom;
    CameFrom.Add(Start, FCellPos{-1, -1});

    while (!Queue.IsEmpty())
    {
        FCellPos Current;
        Queue.Dequeue(Current);

        if (Current == Goal)
        {
            // Reconstruct path
            TArray<FCellPos> Path;
            FCellPos Pos = Goal;
            while (Pos != FCellPos{-1, -1})
            {
                Path.Add(Pos);
                Pos = CameFrom[Pos];
            }
            Algo::Reverse(Path);
            return Path;
        }

        for (const FNeighbor& Dir : Directions)
        {
            int32 NX = Current.X + Dir.DX;
            int32 NY = Current.Y + Dir.DY;
            FCellPos Next{NX, NY};
            if (NX >= 0 && NX < GridSizeX && NY >= 0 && NY < GridSizeY && !CameFrom.Contains(Next) && !IsEdgeFencedBetweenCells(Current.X, Current.Y, NX, NY))
            {
                Queue.Enqueue(Next);
                CameFrom.Add(Next, Current);
            }
        }
    }

    return TArray<FCellPos>();  // No path
}

TArray<FCellPos> ASimulationController::GetHumanPositions() const
{
    TArray<FCellPos> Positions;
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            if (GridOccupants[Y][X] == ECellType::Human)
            {
                Positions.Add({X, Y});
            }
        }
    }
    return Positions;
}

FCellPos ASimulationController::GetRandomEmptyCell() const
{
    TArray<FCellPos> EmptyCells;
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            if (GridOccupants[Y][X] == ECellType::Empty)
            {
                EmptyCells.Add({X, Y});
            }
        }
    }
    if (EmptyCells.IsEmpty()) return {-1, -1};
    int32 RandIndex = FMath::RandRange(0, EmptyCells.Num() - 1);
    return EmptyCells[RandIndex];
}