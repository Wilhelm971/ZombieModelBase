// Copyright University of Inland Norway. All Rights Reserved.

#include "ZombieManager.h"
#include "GridManager.h"
#include "Zombie.h"
#include "Human.h"
#include "Kismet/KismetMathLibrary.h"

AZombieManager::AZombieManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AZombieManager::BeginPlay()
{
    Super::BeginPlay();
}

void AZombieManager::Initialize(AGridManager* InGridManager, const TArray<AZombie*>& StartingZombies)
{
    GridManager = InGridManager;
    AllZombies = StartingZombies;
}

void AZombieManager::ExecuteZombiePhase()
{
    if (!GridManager) return;

    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return;

    TArray<AZombie*> ActiveZombies = GetShuffledZombies();

    for (AZombie* Z : ActiveZombies)
    {
        if (Humans.Num() == 0) break;
        TryMoveAndBite(Z);
    }

    UpdateBittenTimers();
}

bool AZombieManager::IsWinConditionMet() const
{
    if (!GridManager) return false;

    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return false;

    for (AZombie* Z : AllZombies)
    {
        FIntPoint ZGrid = WorldToGrid(Z->GetActorLocation());
        for (FIntPoint H : Humans)
        {
            if (CanZombieReachHuman(ZGrid, H))
                return false;
        }
    }

    return BittenHumans.Num() == 0;
}

// === Core Helpers ===

FIntPoint AZombieManager::WorldToGrid(FVector WorldLocation) const
{
    const float TileSize = 100.f;
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / TileSize),
        FMath::FloorToInt(WorldLocation.Y / TileSize)
    );
}

FVector AZombieManager::GridToWorld(FIntPoint GridPos) const
{
    const float TileSize = 200.f;
    return FVector(GridPos.X * TileSize + TileSize * 0.5f, GridPos.Y * TileSize + TileSize * 0.5f, 50.f);
}

bool AZombieManager::CanZombieReachHuman(FIntPoint Start, FIntPoint Goal) const
{
    if (!GridManager->IsValidCell(Start.X, Start.Y) || !GridManager->IsValidCell(Goal.X, Goal.Y))
        return false;

    TQueue<FIntPoint> Queue;
    TSet<FIntPoint> Visited;
    TMap<FIntPoint, FIntPoint> CameFrom;

    Queue.Enqueue(Start);
    Visited.Add(Start);

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        if (Current == Goal)
            return true;

        static const FIntPoint Directions[4] = { {-1,0}, {1,0}, {0,-1}, {0,1} };

        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint Next = Current + Dir;

            if (GridManager->IsValidCell(Next.X, Next.Y) &&
                !Visited.Contains(Next) &&
                !GridManager->IsEdgeBlockedByFence(Current.X, Current.Y, Next.X, Next.Y))
            {
                Queue.Enqueue(Next);
                Visited.Add(Next);
                CameFrom.Add(Next, Current);
            }
        }
    }
    return false;
}

TArray<AZombie*> AZombieManager::GetShuffledZombies() const
{
    TArray<AZombie*> Copy = AllZombies;
    for (int32 i = Copy.Num() - 1; i > 0; --i)
    {
        Copy.Swap(i, FMath::RandRange(0, i));
    }
    Copy.SetNum(FMath::Min(ZombiesPerTurn, Copy.Num()));
    return Copy;
}

bool AZombieManager::TryMoveAndBite(AZombie* Zombie)
{
    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return false;

    FIntPoint ZPos = WorldToGrid(Zombie->GetActorLocation());

    FIntPoint BestTarget = FIntPoint(-1, -1);
    int32 BestDistance = MAX_int32;

    for (FIntPoint HumanPos : Humans)
    {
        if (CanZombieReachHuman(ZPos, HumanPos))
        {
            // Simple Manhattan for tiebreaker
            int32 Dist = FMath::Abs(HumanPos.X - ZPos.X) + FMath::Abs(HumanPos.Y - ZPos.Y);
            if (Dist < BestDistance)
            {
                BestDistance = Dist;
                BestTarget = HumanPos;
            }
        }
    }

    if (BestTarget == FIntPoint(-1, -1)) return false;

    // === Build path using BFS ===
    TMap<FIntPoint, FIntPoint> CameFrom;
    TQueue<FIntPoint> Queue;
    TSet<FIntPoint> Visited;

    Queue.Enqueue(ZPos);
    Visited.Add(ZPos);

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        if (Current == BestTarget) break;

        static const FIntPoint Dirs[4] = { {-1,0}, {1,0}, {0,-1}, {0,1} };
        for (const FIntPoint& Dir : Dirs)
        {
            FIntPoint Next = Current + Dir;
            if (GridManager->IsValidCell(Next.X, Next.Y) &&
                !Visited.Contains(Next) &&
                !GridManager->IsEdgeBlockedByFence(Current.X, Current.Y, Next.X, Next.Y))
            {
                Queue.Enqueue(Next);
                Visited.Add(Next);
                CameFrom.Add(Next, Current);
            }
        }
    }

    TArray<FIntPoint> Path = ReconstructPathFromBFS(CameFrom, BestTarget);
    if (Path.Num() < 2) return false;

    // Send path to zombie BP
    Zombie->OnMoveAlongPath(Path);

    // Register bite
    AHuman* Human = GetHumanAtGridPos(BestTarget);
    FBittenHuman B;
    B.GridPos = BestTarget;
    B.HumanActor = Human;
    B.TurnsLeft = 15;
    BittenHumans.Add(B);

    return true;
}

TArray<FIntPoint> AZombieManager::ReconstructPathFromBFS(const TMap<FIntPoint, FIntPoint>& CameFrom, FIntPoint End) const
{
    TArray<FIntPoint> Path;
    FIntPoint Current = End;
    while (CameFrom.Contains(Current))
    {
        Path.Insert(Current, 0);
        Current = CameFrom[Current];
    }
    Path.Insert(WorldToGrid(GetActorLocation()), 0); // fallback
    return Path;
}

void AZombieManager::UpdateBittenTimers()
{
    for (int32 i = BittenHumans.Num() - 1; i >= 0; --i)
    {
        if (--BittenHumans[i].TurnsLeft <= 0)
        {
            TurnHumanIntoZombie(BittenHumans[i]);
            BittenHumans.RemoveAt(i);
        }
    }
}

void AZombieManager::TurnHumanIntoZombie(const FBittenHuman& Data)
{
    if (Data.HumanActor)
    {
        Data.HumanActor->Destroy();
    }

    if (ZombieClass)
    {
        FVector SpawnLoc = GridToWorld(Data.GridPos);
        if (AZombie* NewZ = GetWorld()->SpawnActor<AZombie>(ZombieClass, SpawnLoc, FRotator::ZeroRotator))
        {
            AllZombies.Add(NewZ);
        }
    }
}

// === YOU MUST IMPLEMENT THESE TWO FUNCTIONS ===
TArray<FIntPoint> AZombieManager::GetCurrentHumanPositions() const
{
    // Return positions of all alive, unbitten humans
    // Example: loop through your GameMode's HumanArray
    return TArray<FIntPoint>();
}

AHuman* AZombieManager::GetHumanAtGridPos(FIntPoint Pos) const
{
    // Return the AHuman* at this grid position
    return nullptr;
}