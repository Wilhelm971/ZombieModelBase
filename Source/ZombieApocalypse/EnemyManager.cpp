#include "EnemyManager.h"
#include "GridManager.h"  // Your UGridManager with pathfinding
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AEnemyManager::AEnemyManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyManager::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyManager::InitializeManager(UGridManager* InGridManager, TArray<AZombie*> InZombies, int32 InZombiesPerTurn)
{
    GridManager = InGridManager;
    ZombiesPerTurn = InZombiesPerTurn;
    AllZombies = InZombies;
    ZombiePositions.Empty();
    BittenHumans.Empty();

    // Cache initial zombie positions
    for (AZombie* Zombie : AllZombies)
    {
        if (Zombie && GridManager)
        {
            FIntPoint Pos = GridManager->ActorToGridPos(Zombie->GetActorLocation());
            ZombiePositions.Add(Pos);
        }
    }
}

void AEnemyManager::ExecuteZombiePhase()
{
    if (!GridManager || AllZombies.Num() == 0) return;

    // 1. Get current unbitten humans from GridManager (your impl: query living humans)
    CurrentUnbittenHumanPositions = GridManager->GetUnbittenHumanPositions();  // ADD THIS TO GRIDMANAGER!

    // Early GameOver check
    if (CurrentUnbittenHumanPositions.Num() == 0)
    {
        OnGameOver.Broadcast();
        return;
    }

    // 2. Shuffle & select zombies
    TArray<AZombie*> SelectedZombies = GetShuffledZombiesSubset();

    // 3. Sequential bites: each picks from *remaining* unbitten
    for (AZombie* Zombie : SelectedZombies)
    {
        if (TryBiteClosestHuman(Zombie))
        {
            // Remove bitten from current list (GridManager updates on next query)
            CurrentUnbittenHumanPositions.Remove(GridManager->GetLastBittenPos());  // Or track locally
        }
    }

    // 4. Update bitten timers → turn humans
    UpdateBittenHumanTimers();
}

bool AEnemyManager::CheckEndGameConditions()
{
    if (!GridManager) return false;

    // Get fresh unbitten humans
    TArray<FIntPoint> UnbittenHumans = GridManager->GetUnbittenHumanPositions();
    if (UnbittenHumans.Num() == 0)
    {
        OnGameOver.Broadcast();
        return true;
    }

    // Get all zombie positions
    TArray<FIntPoint> CurrentZombiePoss = GetAllZombiePositions();

    // Win: No zombies can reach ANY unbitten AND no bitten pending
    bool ZombiesBlocked = !GridManager->CanAnyZombieReachHumans(UnbittenHumans, CurrentZombiePoss);
    bool NoBittenPending = BittenHumans.Num() == 0;

    if (ZombiesBlocked && NoBittenPending)
    {
        OnGameWin.Broadcast();
        return true;
    }

    return false;
}

TArray<FIntPoint> AEnemyManager::GetAllZombiePositions() const
{
    TArray<FIntPoint> Positions;
    for (AZombie* Zombie : AllZombies)
    {
        if (Zombie && GridManager)
        {
            Positions.Add(GridManager->ActorToGridPos(Zombie->GetActorLocation()));
        }
    }
    return Positions;
}

TArray<AZombie*> AEnemyManager::GetShuffledZombiesSubset() const
{
    TArray<AZombie*> Shuffled = AllZombies;
    for (int32 i = Shuffled.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Shuffled.Swap(i, j);
    }
    Shuffled.SetNum(FMath::Min(ZombiesPerTurn, Shuffled.Num()));
    return Shuffled;
}

bool AEnemyManager::TryBiteClosestHuman(AZombie* Zombie)
{
    if (!Zombie || !GridManager || CurrentUnbittenHumanPositions.Num() == 0) return false;

    FIntPoint ZombiePos = GetZombieGridPos(Zombie);

    // GridManager: Find closest reachable unbitten human
    FIntPoint TargetHuman = GridManager->FindClosestReachableHumanFromPos(ZombiePos, CurrentUnbittenHumanPositions);
    if (TargetHuman == FIntPoint(-1, -1)) return false;

    // Get full path
    TArray<FIntPoint> Path = GridManager->GetShortestPathBetween(ZombiePos, TargetHuman);
    if (Path.Num() == 0) return false;

    // BP Event: Move zombie + anim + bite VFX
    OnMoveZombieToPath.Broadcast(Zombie, Path);

    // Mark bitten (assume GridManager or HumanActor handles "bitten" flag)
    // Add to bitten list (HumanActor ref from GridManager?)
    FBittenHumanData BittenData;
    BittenData.Position = TargetHuman;
    BittenData.HumanActor = GridManager->GetHumanActorAtPos(TargetHuman);  // ADD TO GRIDMANAGER!
    BittenData.TurnsUntilTurn = BittenTurnDelay;
    BittenHumans.Add(BittenData);

    return true;
}

void AEnemyManager::UpdateBittenHumanTimers()
{
    TArray<int32> ToRemoveIndices;

    for (int32 i = 0; i < BittenHumans.Num(); ++i)
    {
        FBittenHumanData& Data = BittenHumans[i];
        Data.TurnsUntilTurn--;
        if (Data.TurnsUntilTurn <= 0)
        {
            TurnBittenHumanIntoZombie(Data);
            ToRemoveIndices.Add(i);
            OnZombieTurned.Broadcast(Data.Position);
        }
    }

    // Remove turned (backwards to avoid invalidation)
    for (int32 i = ToRemoveIndices.Num() - 1; i >= 0; --i)
    {
        BittenHumans.RemoveAt(ToRemoveIndices[i]);
    }
}

void AEnemyManager::TurnBittenHumanIntoZombie(const FBittenHumanData& BittenData)
{
    if (!GridManager || !ZombieClass) return;

    // Destroy/replace mesh → spawn zombie
    if (BittenData.HumanActor)
    {
        BittenData.HumanActor->Destroy();
    }

    FVector WorldPos = GridManager->GridToWorldPos(BittenData.Position);
    AZombie* NewZombie = GetWorld()->SpawnActor<AZombie>(ZombieClass, WorldPos, FRotator::ZeroRotator);
    if (NewZombie)
    {
        AllZombies.Add(NewZombie);
        ZombiePositions.Add(BittenData.Position);
    }
}

FIntPoint AEnemyManager::GetZombieGridPos(const AZombie* Zombie) const
{
    return GridManager ? GridManager->ActorToGridPos(Zombie->GetActorLocation()) : FIntPoint::ZeroValue;
}