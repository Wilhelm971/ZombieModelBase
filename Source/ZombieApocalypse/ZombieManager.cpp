#include "ZombieManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AZombieManager::AZombieManager()
{
    PrimaryActorTick.bCanEverTick = false; // Turn-based, no tick
}

void AZombieManager::BeginPlay()
{
    Super::BeginPlay();

    SpawnInitialNPCs();
}

void AZombieManager::SpawnInitialNPCs()
{
    if (!GridManager || !NPCClass) return;

    const int32 GridSizeX = 10; // Assuming 10x10 grid
    const int32 GridSizeY = 10;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bNoFail = true;

    // Spawn humans on all cells
    for (int32 X = 0; X < GridSizeX; ++X)
    {
        for (int32 Y = 0; Y < GridSizeY; ++Y)
        {
            FVector SpawnPos = GridManager->GetCellCenterWorldPos(X, Y);
            ANonPlayerCharacters* Human = GetWorld()->SpawnActor<ANonPlayerCharacters>(NPCClass, SpawnPos, FRotator::ZeroRotator, SpawnParams);
            if (Human)
            {
                Human->GridPosition = FIntPoint(X, Y);

                GridManager->Grid[Y * GridSizeX + X].bHasHuman = true;
                AllNPCs.Add(Human);
            }
        }
    }

    // Spawn zombie in center
    int32 CenterX = 5;
    int32 CenterY = 5;
    FVector ZombieSpawnPos = GridManager->GetCellCenterWorldPos(CenterX, CenterY);
    ANonPlayerCharacters* Zombie = GetWorld()->SpawnActor<ANonPlayerCharacters>(NPCClass, ZombieSpawnPos, FRotator::ZeroRotator, SpawnParams);
    if (Zombie)
    {
        Zombie->SetState(EState::Zombie);
        Zombie->GridPosition = FIntPoint(CenterX, CenterY);

        AllNPCs.Add(Zombie);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Zombie"));
    }
}

void AZombieManager::ExecuteTurn()
{
    UpdateBittenTimers();

    TArray<ANonPlayerCharacters*> ActiveZombies = GetShuffledZombies();

    for (ANonPlayerCharacters* Z : ActiveZombies)
    {
        if (AllowedBitesThisTurn <= 0) break;

        TryMoveAndBite(Z);

        AllowedBitesThisTurn--;
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

TArray<ANonPlayerCharacters*> AZombieManager::GetShuffledZombies() const
{
    TArray<ANonPlayerCharacters*> Zombies;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Zombie)
            Zombies.Add(NPC);
    }

    for (int32 i = 0; i < Zombies.Num(); ++i)
    {
        int32 SwapIdx = FMath::RandRange(0, Zombies.Num() - 1);
        Zombies.Swap(i, SwapIdx);
    }

    return Zombies;
}

TArray<FIntPoint> AZombieManager::GetCurrentHumanPositions() const
{
    TArray<FIntPoint> Humans;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human)
            Humans.Add(NPC->GridPosition);
    }
    return Humans;
}

ANonPlayerCharacters* AZombieManager::GetHumanAtGridPos(const FIntPoint& Pos) const
{
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human && NPC->GridPosition == Pos)
            return NPC;
    }
    return nullptr;
}

bool AZombieManager::TryMoveAndBite(ANonPlayerCharacters* Zombie)
{
    if (!GridManager || !Zombie) return false;

    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return false;

    FIntPoint ZPos = Zombie->GridPosition;

    FIntPoint Best(-1, -1);
    int32 BestDist = MAX_int32;

    for (FIntPoint HPos : Humans)
    {
        TArray<FGridNode> Path;
        FGridNode Start(HPos.X, HPos.Y);
        FGridNode End(HPos.X, HPos.Y);

        // Path from Zombie to Human
        if (!GridManager->FindPath(FGridNode(ZPos.X, ZPos.Y), FGridNode(HPos.X, HPos.Y), Path)) 
        {
            UE_LOG(LogTemp, Warning, TEXT("No path to human at %d,%d"), HPos.X, HPos.Y);
            continue;
        }

        int32 Dist = Path.Num();
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Best = HPos;
        }
    }

    if (Best == FIntPoint(-1, -1)) return false;

    // Get path
    TArray<FGridNode> Path;
    if (!GridManager->FindPath(FGridNode(ZPos.X, ZPos.Y), FGridNode(Best.X, Best.Y), Path))
        return false;

    TArray<FVector> VPath;
    for (auto& Node : Path)
    {
        FVector Location = GridManager->GetCellCenterWorldPos(Node.X, Node.Y);
        VPath.Add(Location);
    }

    // give the zombie a movement path
    Zombie->MoveAlongWorldPath(VPath);

    ANonPlayerCharacters* Human = GetHumanAtGridPos(Best);
    if (Human)
    {
        Human->SetState(EState::Bitten);
        UE_LOG(LogTemp, Warning, TEXT("HumanBitten"));
        FBittenNPC B;
        B.NPC = Human;
        B.GridPos = Best;
        B.TurnsLeft = 15;
        BittenNPCs.Add(B);
    }

    return true;
}

int32 AZombieManager::GetSusceptibleCount() const
{
    int32 Count = 0;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human)
            Count++;
    }
    return Count;
}

int32 AZombieManager::GetBittenCount() const
{
    int32 Count = 0;
    for (const FBittenNPC& Entry : BittenNPCs)
    {
        if (Entry.NPC && Entry.NPC->GetState() == EState::Bitten)
            Count++;
    }
    return Count;
}

int32 AZombieManager::GetZombieCount() const
{
    int32 Count = 0;
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Zombie)
            Count++;
    }
    return Count;
}
