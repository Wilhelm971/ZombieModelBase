/*
#include "ZombieManager.h"
#include "Kismet/KismetMathLibrary.h"

AZombieManager::AZombieManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AZombieManager::BeginPlay()
{
    Super::BeginPlay();
}

void AZombieManager::Initialize(AGridManager* InGrid)
{
    GridManager = InGrid;
}

void AZombieManager::SpawnInitialPopulation()
{
    if (!GridManager || !BeingClass) return;

    AllBeings.Empty();

    // ----- Spawn Humans -----
    for (int32 X = 0; X < GridManager->GridSize; X++)
    {
        for (int32 Y = 0; Y < GridManager->GridSize; Y++)
        {
            FVector SpawnLoc = GridManager->GetCellCenterWorldPos(X, Y);

            ABeing* H = GetWorld()->SpawnActor<ABeing>(BeingClass, SpawnLoc, FRotator::ZeroRotator);
            H->BeingType = EBeingType::Human;
            H->SetGridPosition(X, Y);

            AllBeings.Add(H);
        }
    }

    // ----- Spawn 1 central zombie -----
    int32 Mid = GridManager->GridSize / 2;
    FVector ZLoc = GridManager->GetCellCenterWorldPos(Mid, Mid);

    ABeing* Z = GetWorld()->SpawnActor<ABeing>(BeingClass, ZLoc, FRotator::ZeroRotator);
    Z->BeingType = EBeingType::Zombie;
    Z->SetGridPosition(Mid, Mid);

    AllBeings.Add(Z);
}

void AZombieManager::ExecuteZombieTurn()
{
    TArray<ABeing*> Zombies = GetZombies();
    if (Zombies.Num() == 0) return;

    // Shuffle
    for (int32 i = Zombies.Num() - 1; i > 0; --i)
        Zombies.Swap(i, FMath::RandRange(0, i));

    Zombies.SetNum(FMath::Min(ZombiesPerTurn, Zombies.Num()));

    for (ABeing* Z : Zombies)
        TryMoveAndBite(Z);

    UpdateBitten();
}

bool AZombieManager::IsWinConditionMet() const
{
    // No humans left
    for (ABeing* B : AllBeings)
        if (B && B->BeingType == EBeingType::Human)
            return false;

    return BittenList.Num() == 0;
}

// =============================================================
// Helper Functions
// =============================================================

FVector AZombieManager::GridToWorld(FIntPoint Grid) const
{
    return GridManager->GetCellCenterWorldPos(Grid.X, Grid.Y);
}

FIntPoint AZombieManager::WorldToGrid(FVector W) const
{
    const float Size = GridManager->CellSize;
    FVector L = W - GridManager->GetActorLocation();

    return FIntPoint(
        FMath::FloorToInt(L.X / Size),
        FMath::FloorToInt(L.Y / Size)
    );
}

TArray<ABeing*> AZombieManager::GetZombies() const
{
    TArray<ABeing*> Out;
    for (ABeing* B : AllBeings)
        if (B && B->BeingType == EBeingType::Zombie)
            Out.Add(B);
    return Out;
}

TArray<FIntPoint> AZombieManager::GetHumanPositions() const
{
    TArray<FIntPoint> Out;

    for (ABeing* B : AllBeings)
    {
        if (!B) continue;
        if (B->BeingType != EBeingType::Human) continue;

        FIntPoint P = B->GetGridPosition();
        Out.Add(P);
    }
    return Out;
}

ABeing* AZombieManager::GetHumanAt(FIntPoint Pos) const
{
    for (ABeing* B : AllBeings)
    {
        if (B && B->BeingType == EBeingType::Human)
        {
            if (B->GetGridPosition() == Pos)
                return B;
        }
    }
    return nullptr;
}

bool AZombieManager::TryMoveAndBite(ABeing* Zombie)
{
    FIntPoint ZPos = Zombie->GetGridPosition();
    TArray<FIntPoint> Humans = GetHumanPositions();

    if (Humans.Num() == 0) return false;

    // Pick closest reachable target
    FIntPoint Best = FIntPoint(-1, -1);
    int32 BestDist = MAX_int32;

    for (const FIntPoint& H : Humans)
    {
        if (!CanReachHuman(ZPos, H)) continue;

        int32 D = FMath::Abs(H.X - ZPos.X) + FMath::Abs(H.Y - ZPos.Y);
        if (D < BestDist)
        {
            BestDist = D;
            Best = H;
        }
    }

    if (Best.X < 0) return false;

    // Build path
    TArray<FIntPoint> Path;
    if (!BuildBFSPath(ZPos, Best, Path)) return false;

    Zombie->OnMoveAlongPath(Path);

    // Register bite
    ABeing* Human = GetHumanAt(Best);
    if (Human)
    {
        FBittenEntry B;
        B.GridPos = Best;
        B.HumanActor = Human;
        B.TurnsLeft = 15;
        BittenList.Add(B);
    }

    return true;
}

bool AZombieManager::CanReachHuman(FIntPoint Start, FIntPoint Goal) const
{
    return BuildBFSPath(Start, Goal, TArray<FIntPoint>());
}

bool AZombieManager::BuildBFSPath(FIntPoint Start, FIntPoint Goal, TArray<FIntPoint>& OutPath) const
{
    TArray<FGridNode> Path;
    bool Found = GridManager->FindPath(
        FGridNode(Start.X, Start.Y),
        FGridNode(Goal.X, Goal.Y),
        Path
    );

    if (!Found) return false;

    OutPath.Empty();
    for (auto& N : Path)
        OutPath.Add(FIntPoint(N.X, N.Y));

    return true;
}

void AZombieManager::UpdateBitten()
{
    for (int32 i = BittenList.Num() - 1; i >= 0; i--)
    {
        if (--BittenList[i].TurnsLeft <= 0)
        {
            TurnHumanIntoZombie(BittenList[i]);
            BittenList.RemoveAt(i);
        }
    }
}

void AZombieManager::TurnHumanIntoZombie(const FBittenEntry& Data)
{
    if (Data.HumanActor)
    {
        AllBeings.Remove(Data.HumanActor);
        Data.HumanActor->Destroy();
    }

    // Spawn new zombie
    FVector SpawnLoc = GridToWorld(Data.GridPos);
    ABeing* Z = GetWorld()->SpawnActor<ABeing>(BeingClass, SpawnLoc, FRotator::ZeroRotator);
    Z->BeingType = EBeingType::Zombie;
    Z->SetGridPosition(Data.GridPos.X, Data.GridPos.Y);

    AllBeings.Add(Z);
}
*/