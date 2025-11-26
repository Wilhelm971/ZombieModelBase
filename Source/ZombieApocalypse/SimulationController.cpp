// Copyright University of Inland Norway

#include "SimulationController.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include <algorithm>  // <- FIX: For std::sort

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();

    // Find GridManager
    GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("SimulationController: GridManager not found!"));
    }

    // Initialize grid occupants
    if (GridManager)
    {
        // All cells start with 1 susceptible
        for (auto& Cell : GridManager->Grid)
        {
            Cell.NumSusceptible = 1;
        }

        // Place initial zombie in random cell (add, not replace)
        int RandX = FMath::RandRange(0, AGridManager::GridSize - 1);
        int RandY = FMath::RandRange(0, AGridManager::GridSize - 1);
        int Index = GridManager->GetGridIndex(RandX, RandY);
        GridManager->Grid[Index].NumZombies = 1;

        Susceptible = 100;
        Zombies = 1;
        Bitten = 0;
    }

    if (!PopulationDensityEffectTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable is not assigned!"));
    }
    else
    {
        ReadDataFromTableToVectors();
    }
}

void ASimulationController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (TimeStepsFinished < 100)
    {
        AccumulatedTime += DeltaTime;
        if (AccumulatedTime >= SimulationStepTime && !bIsPlayerTurn)
        {
            AccumulatedTime = 0.f;
            PerformSimulationStep();
            ++TimeStepsFinished;
            bIsPlayerTurn = true;
            if (bShouldDebug)
            {
                UE_LOG(LogTemp, Log, TEXT("Day %d | S:%d B:%d Z:%d"),
                    TimeStepsFinished, Susceptible, Bitten, Zombies);
            }
        }
    }
}

void ASimulationController::EndPlayerTurn()
{
    bIsPlayerTurn = false;
}

// Function to read data from Unreal DataTable into the graphPts vector
void ASimulationController::ReadDataFromTableToVectors()
{
    if (bShouldDebug) UE_LOG(LogTemp, Log, TEXT("ReadDataFromTableToVectors"));

    const TArray<FName> RowNames = PopulationDensityEffectTable->GetRowNames();

    for (int32 Idx = 0; Idx < RowNames.Num(); ++Idx)
    {
        const FPopulationDensityEffect* Row = PopulationDensityEffectTable->FindRow<FPopulationDensityEffect>(RowNames[Idx], TEXT(""));
        if (Row)
        {
            graphPts.emplace_back(Row->PopulationDensity, Row->NormalPopulationDensity);

            if (bShouldDebug)
            {
                const auto& P = graphPts.back();
                UE_LOG(LogTemp, Warning, TEXT("Row %d -> (%.3f , %.3f)"), Idx, P.first, P.second);
            }
        }
    }
}

float ASimulationController::GraphLookup(float X) const
{
    if (graphPts.empty()) return 0.f;

    if (X <= graphPts.front().first) return graphPts.front().second;
    if (X >= graphPts.back().first) return graphPts.back().second;

    for (size_t i = 1; i < graphPts.size(); ++i)
    {
        if (X <= graphPts[i].first)
        {
            const float x0 = graphPts[i - 1].first;
            const float x1 = graphPts[i].first;
            const float y0 = graphPts[i - 1].second;
            const float y1 = graphPts[i].second;
            const float t = (X - x0) / (x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }

    return graphPts.back().second;
}

int ASimulationController::ConveyorContent() const
{
    int Sum = 0;
    for (const FConveyorBatch& Batch : Conveyor)
        Sum += Batch.AmountOfPeople;
    return Sum;
}

void ASimulationController::PerformSimulationStep()
{
    // 1. Advance conveyor (bitten -> zombies, spatial)
    std::vector<FConveyorBatch> NextConveyor;
    NextConveyor.reserve(Conveyor.size());

    int BecomingInfected = 0;
    for (FConveyorBatch& Batch : Conveyor)
    {
        Batch.RemainingDays -= 1.f;
        if (Batch.RemainingDays <= 0.f)
        {
            BecomingInfected += Batch.AmountOfPeople;
            int Index = GridManager->GetGridIndex(Batch.LocationX, Batch.LocationY);
            GridManager->Grid[Index].NumBitten -= Batch.AmountOfPeople;
            GridManager->Grid[Index].NumZombies += Batch.AmountOfPeople;
        }
        else
        {
            NextConveyor.push_back(std::move(Batch));
        }
    }
    Conveyor.swap(NextConveyor);
    Zombies += BecomingInfected;

    // 2. Zombie actions (movement and biting)
    TArray<TArray<FGridNode>> Regions = GridManager->GetConnectedComponents();
    for (const TArray<FGridNode>& RegionCells : Regions)
    {
        // Calculate local stats
        int LocalSus = 0, LocalBitten = 0, LocalZombies = 0;
        for (const FGridNode& Node : RegionCells)
        {
            int Index = GridManager->GetGridIndex(Node.X, Node.Y);
            LocalSus += GridManager->Grid[Index].NumSusceptible;
            LocalBitten += GridManager->Grid[Index].NumBitten;
            LocalZombies += GridManager->Grid[Index].NumZombies;
        }
        int LocalNon = LocalSus + LocalBitten;
        if (LocalNon == 0 || LocalZombies == 0) continue;

        float CellArea = LandArea / (AGridManager::GridSize * AGridManager::GridSize);
        float LocalDensity = static_cast<float>(LocalNon) / (RegionCells.Num() * CellArea);
        float X = LocalDensity / NormalPopulationDensity;
        float DensityEffect = GraphLookup(X);

        int NumActing = FMath::RoundToInt(LocalZombies * NormalNumberOfBites * DensityEffect);

        // Collect zombie locations and their min dist to human
        std::vector<std::pair<int, FGridNode>> SortedZombieCells;
        for (const FGridNode& Node : RegionCells)
        {
            int Index = GridManager->GetGridIndex(Node.X, Node.Y);
            if (GridManager->Grid[Index].NumZombies > 0)
            {
                FGridNode Closest;
                int Dist = GridManager->FindMinDistToHuman(Node, Closest);
                if (Dist < 999)
                {
                    SortedZombieCells.emplace_back(Dist, Node);
                }
            }
        }
        std::sort(SortedZombieCells.begin(), SortedZombieCells.end());

        // Perform actions for acting zombies
        int Acted = 0;
        for (const auto& Pair : SortedZombieCells)
        {
            FGridNode ZLoc = Pair.second;
            int ZIndex = GridManager->GetGridIndex(ZLoc.X, ZLoc.Y);
            while (Acted < NumActing && GridManager->Grid[ZIndex].NumZombies > 0)
            {
                // Find closest human cell
                FGridNode Target;
                int Dist = GridManager->FindMinDistToHuman(ZLoc, Target);
                if (Dist >= 999) break;

                // Move zombie
                GridManager->Grid[ZIndex].NumZombies--;
                int TIndex = GridManager->GetGridIndex(Target.X, Target.Y);
                GridManager->Grid[TIndex].NumZombies++;

                // Bite
                int TSus = GridManager->Grid[TIndex].NumSusceptible;
                int TBitten = GridManager->Grid[TIndex].NumBitten;
                int TNon = TSus + TBitten;
                if (TNon > 0)
                {
                    float ProbSus = static_cast<float>(TSus) / TNon;
                    if (FMath::FRand() < ProbSus)
                    {
                        GridManager->Grid[TIndex].NumSusceptible--;
                        GridManager->Grid[TIndex].NumBitten++;
                        Conveyor.push_back({1, DaysToBecomeInfectedFromBite, Target.X, Target.Y});
                        Susceptible--;
                        Bitten++;
                    }
                }

                Acted++;
            }
        }
    }

    // 3. Update globals (Bitten from conveyor for consistency)
    Bitten = ConveyorContent();
    UpdateGlobalCounts(); // Recalc Sus/Zombies if needed (paranoia)
}

void ASimulationController::UpdateGlobalCounts()
{
    Susceptible = 0;
    Zombies = 0;
    for (const auto& Cell : GridManager->Grid)
    {
        Susceptible += Cell.NumSusceptible;
        Zombies += Cell.NumZombies;
    }
}