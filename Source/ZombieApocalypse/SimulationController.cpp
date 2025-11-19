// Copyright University of Inland Norway

#include "SimulationController.h"
#include "Math/UnrealMathUtility.h"

ASimulationController::ASimulationController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASimulationController::BeginPlay()
{
    Super::BeginPlay();

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

    AccumulatedTime += DeltaTime;

    // One full simulation day passed
    if (AccumulatedTime >= SimulationStepTime)
    {
        AccumulatedTime = 0.f;
        PerformSimulationStep();

        ++TimeStepsFinished;
        if (bShouldDebug)
        {
            UE_LOG(LogTemp, Log, TEXT("Day %d | S:%.2f B:%.2f Z:%.2f"),
                TimeStepsFinished, Susceptible, Bitten, Zombies);
        }
    }  
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
    if (graphPts.front().first) return 0.f;

    if (X <= graphPts.front().first)    return graphPts.front().second;
    if (X >= graphPts.back().first)     return graphPts.back().second;

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

float ASimulationController::ConveyorContent() const
{
    float Sum = 0.f;
    for (const FConveyorBatch& Batch : Conveyor)
        Sum += Batch.AmountOfPeople;
    return Sum;
}

void ASimulationController::PerformSimulationStep()
{
    // 1. - Update bitten
    Bitten = ConveyorContent();

    // 2. - Auxiliaries
    const float NonZombiePopulation = Bitten + Susceptible;
    const float PopulationDensity   = NonZombiePopulation / LandArea;
    const float X                   = PopulationDensity / NormalPopulationDensity;

    const float DensityEffect = GraphLookup(X);
    const float BitesPerZombieDay = NormalNumberOfBites * DensityEffect;
    
    const float TotalBittenPerDay = FMath::RoundToFloat(Zombies * BitesPerZombieDay);

    const float Denom = FMath::Max(NonZombiePopulation, 1.f);
    const float BitesOnSusceptible = FMath::RoundToFloat((Susceptible / Denom) * TotalBittenPerDay);

    // 3. - Getting bitten
    float GettingBitten = FMath::Min(BitesOnSusceptible, FMath::FloorToFloat(Susceptible));

    // 4. - CONVEYOR MECHANICS
    // 4.1 - Advance every batch
    for (FConveyorBatch& Batch : Conveyor)
    {
        Batch.RemainingDays -= 1.f;
    }

    // 4.2 - Separate finished batches -> raw outflow
    std::vector<FConveyorBatch> NextConveyor;
    NextConveyor.reserve(Conveyor.size());

    float RawOutflowPeople = 0.f;
    for (FConveyorBatch& Batch : Conveyor)
    {
        if (Batch.RemainingDays <= 0.f)
            RawOutflowPeople += Batch.AmountOfPeople;
        else
            NextConveyor.push_back(MoveTemp(Batch));
    }
    
    Conveyor.swap(NextConveyor);

    // 4.3 - inflow
    const float CurrentContent = ConveyorContent();
    const float FreeCapacity = FMath::Max(0.f, BittenCapacity - CurrentContent);
    const float InflowPeople = FMath::Max(0.f, FMath::Min(GettingBitten, FreeCapacity));

    if (InflowPeople > 0.f)
    {
        Conveyor.push_back({ InflowPeople, DaysToBecomeInfectedFromBite });
    }

    // 4.4 - Outflow -> New Zombie
    const float BecomingInfected = RawOutflowPeople;

    // 5 - STOCK UPDATES
    Susceptible = FMath::Max(0.f, Susceptible - GettingBitten);
    Zombies = FMath::Max(0.f, Zombies + BecomingInfected);

    Bitten = ConveyorContent();
}
