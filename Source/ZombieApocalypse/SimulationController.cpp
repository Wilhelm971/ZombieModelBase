// Copyright University of Inland Norway

#include "SimulationController.h"
#include "EdgeActor.h"
#include <cmath>

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = false; // Turn-based, no need for Tick

    // Initialize live stocks
    CurrentSusceptible = InitialSusceptible;
    CurrentZombies = InitialZombies;
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();

    // Checking if the DataTable is assigned
    if (!PopulationDensityEffectTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PopulationDensityEffectTable is not assigned!"));
    }
    else
    {
        // Table found, read data into vector
        ReadDataFromTableToVectors();
    }

    // Calculate max edges: For NxM cells, horizontal edges = (N+1)*M, vertical = N*(M+1)
    MaxEdges = (GridSizeX + 1) * GridSizeY + GridSizeX * (GridSizeY + 1);
    FencedEdges.Init(false, MaxEdges);

    // Spawn the grid
    SpawnGrid();
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

    // 1. Calculate current totals
    CurrentBitten = 0.f;
    for (const FConveyorBatch& Batch : BittenConveyor)
    {
        CurrentBitten += Batch.AmountOfPeople;
    }

    float NonZombiePopulation = CurrentSusceptible + CurrentBitten;
    float PopulationDensity = NonZombiePopulation / LandArea;
    float DensityRatio = PopulationDensity / NormalPopulationDensity;

    float DensityEffect = GraphLookup(DensityRatio);
    float BiteRatePerZombie = NormalBiteRate * DensityEffect * ContainmentEffectMultiplier;
    float PotentialBites = CurrentZombies * BiteRatePerZombie;
    float TotalBites = FMath::RoundToFloat(PotentialBites);

    float Denominator = FMath::Max(NonZombiePopulation, 1.0f);
    float BitesOnSusceptible = FMath::RoundToFloat((CurrentSusceptible / Denominator) * TotalBites);

    float GettingBitten = FMath::Min(BitesOnSusceptible, CurrentSusceptible);

    if (bShouldDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("Day %d: Biting %f susceptible"), CurrentDay, GettingBitten);
    }

    // 2. Advance conveyor age
    for (FConveyorBatch& Batch : BittenConveyor)
    {
        Batch.RemainingDays -= 1.0f;
    }

    // 3. Outflow → new zombies
    float BecomingZombies = 0.f;
    TArray<FConveyorBatch> SurvivingBatches;
    SurvivingBatches.Reserve(BittenConveyor.Num());

    for (const FConveyorBatch& Batch : BittenConveyor)
    {
        if (Batch.RemainingDays <= 0.f)
        {
            BecomingZombies += Batch.AmountOfPeople;
        }
        else
        {
            SurvivingBatches.Add(Batch);
        }
    }
    BittenConveyor = SurvivingBatches;

    // 4. Inflow (capacity limited)
    float CapacityUsed = 0.f;
    for (const FConveyorBatch& Batch : BittenConveyor)
    {
        CapacityUsed += Batch.AmountOfPeople;
    }
    float FreeCapacity = FMath::Max(0.f, BittenCapacity - CapacityUsed);
    float ActualInflow = FMath::Min(GettingBitten, FreeCapacity);

    if (ActualInflow > 0.f)
    {
        FConveyorBatch NewBatch;
        NewBatch.AmountOfPeople = ActualInflow;
        NewBatch.RemainingDays = DaysToBecomeZombie;
        BittenConveyor.Add(NewBatch);
    }

    // 5. Update stocks
    CurrentSusceptible = FMath::Max(0.f, CurrentSusceptible - GettingBitten);
    CurrentZombies += BecomingZombies;

    // Update bitten for next time
    CurrentBitten = 0.f;
    for (const FConveyorBatch& Batch : BittenConveyor)
    {
        CurrentBitten += Batch.AmountOfPeople;
    }

    if (bShouldDebug)
    {
        UE_LOG(LogTemp, Log, TEXT("Day %d: S=%f, B=%f, Z=%f"), CurrentDay, CurrentSusceptible, CurrentBitten, CurrentZombies);
    }

    // Broadcast for HUD/UI
    OnDayAdvanced.Broadcast(CurrentDay);

    // Check win/lose (optional)
    if (CurrentSusceptible <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Humanity lost!"));
    }
    else if (CurrentZombies <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Outbreak contained!"));
    }
}

float ASimulationController::GraphLookup(float X) const
{
    if (graphPts.IsEmpty()) return 1.0f;

    if (X <= graphPts[0].first) return graphPts[0].second;
    if (X >= graphPts.Last().first) return graphPts.Last().second;

    for (int i = 1; i < graphPts.Num(); ++i)
    {
        if (X <= graphPts[i].first)
        {
            float x0 = graphPts[i-1].first;
            float x1 = graphPts[i].first;
            float y0 = graphPts[i-1].second;
            float y1 = graphPts[i].second;
            float t = (X - x0) / (x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }
    return graphPts.Last().second;
}

void ASimulationController::PlaceFence(int32 EdgeID)
{
    if (EdgeID >= 0 && EdgeID < MaxEdges)
    {
        FencedEdges[EdgeID] = true;
        UpdateContainmentEffect();
        if (bShouldDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("Fence placed on edge %d. Containment: %f"), EdgeID, ContainmentEffectMultiplier);
        }
    }
}

void ASimulationController::UpdateContainmentEffect()
{
    int32 NumFenced = 0;
    for (int32 i = 0; i < MaxEdges; ++i)
    {
        if (FencedEdges[i]) NumFenced++;
    }
    // Simple linear: each fence reduces by 0.01, min 0.5
    float Reduction = static_cast<float>(NumFenced) / MaxEdges;
    ContainmentEffectMultiplier = FMath::Clamp(1.0f - Reduction, 0.5f, 1.0f);
}

void ASimulationController::SpawnGrid()
{
    if (!EdgeActorClass) 
    {
        UE_LOG(LogTemp, Error, TEXT("EdgeActorClass not assigned!"));
        return;
    }

    int32 EdgeCounter = 0;
    FVector BaseLocation = GetActorLocation();  // Spawn relative to controller

    // Spawn horizontal edges
    for (int32 Y = 0; Y <= GridSizeY; ++Y)
    {
        for (int32 X = 0; X < GridSizeX; ++X)
        {
            FVector Location = BaseLocation + FVector(X * CellSpacing, Y * CellSpacing, 0.f);
            FRotator Rotation(0.f, 0.f, 0.f);  // Horizontal

            AEdgeActor* NewEdge = GetWorld()->SpawnActor<AEdgeActor>(EdgeActorClass, Location, Rotation);
            if (NewEdge)
            {
                NewEdge->EdgeID = EdgeCounter++;
                NewEdge->SetActorScale3D(FVector(CellSpacing / 100.f, 0.1f, 0.1f));  // Scale to fit cell
            }
        }
    }

    // Spawn vertical edges
    for (int32 Y = 0; Y < GridSizeY; ++Y)
    {
        for (int32 X = 0; X <= GridSizeX; ++X)
        {
            FVector Location = BaseLocation + FVector(X * CellSpacing, Y * CellSpacing, 0.f);
            FRotator Rotation(0.f, 90.f, 0.f);  // Vertical

            AEdgeActor* NewEdge = GetWorld()->SpawnActor<AEdgeActor>(EdgeActorClass, Location, Rotation);
            if (NewEdge)
            {
                NewEdge->EdgeID = EdgeCounter++;
                NewEdge->SetActorScale3D(FVector(0.1f, CellSpacing / 100.f, 0.1f));  // Scale to fit
            }
        }
    }
}