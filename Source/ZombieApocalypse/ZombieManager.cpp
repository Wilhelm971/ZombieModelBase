#include "ZombieManager.h"
#include "Kismet/GameplayStatics.h"

AZombieManager::AZombieManager()
{
    PrimaryActorTick.bCanEverTick = false; // Turn-based
}

void AZombieManager::BeginPlay()
{
    Super::BeginPlay();

    SpawnInitialNPCs();
}

void AZombieManager::SpawnInitialNPCs()
{
    if (!GridManager) return;

    int32 GridSizeX = 10; // assuming 10x10 grid
    int32 GridSizeY = 10;

    // Spawn humans on all cells
    for (int32 X = 0; X < GridSizeX; ++X)
    {
        for (int32 Y = 0; Y < GridSizeY; ++Y)
        {
            FVector SpawnPos = GridManager->GetCellCenterWorldPos(X, Y);
            ANonPlayerCharacters* Human = GetWorld()->SpawnActor<ANonPlayerCharacters>(HumanClass, SpawnPos, FRotator::ZeroRotator);
            if (Human)
            {
                Human->SetState(EState::Human);
                Human->GridPosition = FIntPoint(X, Y);

                GridManager->Grid[Y * GridSizeX + X].bHasHuman = true;
                AllNPCs.Add(Human);
            }
        }
    }

    // Spawn zombie in the center
    int32 CenterX = GridSizeX / 2;
    int32 CenterY = GridSizeY / 2;
    FVector ZombieSpawnPos = GridManager->GetCellCenterWorldPos(CenterX, CenterY);
    ANonPlayerCharacters* Zombie = GetWorld()->SpawnActor<ANonPlayerCharacters>(ZombieClass, ZombieSpawnPos, FRotator::ZeroRotator);
    if (Zombie)
    {
        Zombie->SetState(EState::Zombie);
        Zombie->GridPosition = FIntPoint(CenterX, CenterY);
        AllNPCs.Add(Zombie);
    }
}



void AZombieManager::ExecuteTurn()
{
    UpdateBittenTimers();

    // Collect all zombies
    TArray<ANonPlayerCharacters*> ActiveZombies;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Zombie)
        {
            ActiveZombies.Add(NPC);
        }
    }

    // Shuffle zombies for random turn order
    for (int32 i = 0; i < ActiveZombies.Num(); ++i)
    {
        int32 SwapIndex = FMath::RandRange(0, ActiveZombies.Num() - 1);
        ActiveZombies.Swap(i, SwapIndex);
    }

    // Each zombie tries to move and bite
    for (ANonPlayerCharacters* Z : ActiveZombies)
    {
        TryMoveAndBite(Z);
    }
}

void AZombieManager::UpdateBittenTimers()
{
    for (int32 i = BittenNPCs.Num() - 1; i >= 0; --i)
    {
        FBittenNPC& B = BittenNPCs[i];
        B.TurnsLeft--;
        if (B.TurnsLeft <= 0 && B.NPC)
        {
            B.NPC->SetState(EState::Zombie);
            BittenNPCs.RemoveAt(i);
        }
    }
}

TArray<FIntPoint> AZombieManager::GetCurrentHumanPositions() const
{
    TArray<FIntPoint> Humans;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human)
        {
            Humans.Add(NPC->GridPosition); // <-- Use stored grid position
        }
    }
    return Humans;
}

ANonPlayerCharacters* AZombieManager::GetHumanAtGridPos(const FIntPoint& Pos) const
{
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human && NPC->GridPosition == Pos)
        {
            return NPC;
        }
    }
    return nullptr;
}

bool AZombieManager::TryMoveAndBite(ANonPlayerCharacters* Zombie)
{
    if (!Zombie || !GridManager) return false;

    FIntPoint ZPos = Zombie->GridPosition; // <-- Use stored grid position
    TArray<FIntPoint> Humans = GetCurrentHumanPositions();

    if (Humans.Num() == 0) return false;

    FIntPoint Best(-1, -1);
    int32 BestDist = MAX_int32;
    TArray<FGridNode> BestPath;

    // Find closest reachable human by actual path
    for (const FIntPoint& HPos : Humans)
    {
        TArray<FGridNode> PathNodes;
        bool bCanReach = GridManager->FindPath(FGridNode(ZPos.X, ZPos.Y), FGridNode(HPos.X, HPos.Y), PathNodes);

        if (!bCanReach || PathNodes.Num() < 2) continue; // Unreachable or already at human

        if (PathNodes.Num() < BestDist)
        {
            BestDist = PathNodes.Num();
            Best = HPos;
            BestPath = PathNodes;
        }
    }

    if (Best == FIntPoint(-1, -1)) return false;

    // Convert path to FIntPoint for movement
    TArray<FIntPoint> Path;
    for (const FGridNode& Node : BestPath)
    {
        Path.Add(FIntPoint(Node.X, Node.Y));
    }

    // Move zombie along path (first step only or full path depending on design)
    //Zombie->OnMoveAlongPath(Path);

    // Bite human if adjacent
    ANonPlayerCharacters* Human = GetHumanAtGridPos(Best);
    if (Human)
    {
        Human->SetState(EState::Bitten);

        FBittenNPC B;
        B.NPC = Human;
        B.GridPos = Best;
        B.TurnsLeft = 15;
        BittenNPCs.Add(B);
    }

    return true;
}
