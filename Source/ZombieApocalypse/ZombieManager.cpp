#include "ZombieManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

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
            ANPC* Human = GetWorld()->SpawnActor<ANPC>(NPCClass, SpawnPos, FRotator::ZeroRotator, SpawnParams);
            if (Human)
            {
                Human->GridPoint = FIntPoint(X, Y);
                Human->ZombieManager = this;

                GridManager->Grid[Y * GridSizeX + X].bHasHuman = true;
                AllNPCs.Add(Human);
            }
        }
    }

    // Spawn zombie in random 4 squares in center
    int32 Random1 = FMath::RandRange(0, 1);
    int32 Random2 = FMath::RandRange(0, 1);

    int32 CenterX = 4 + Random1;
    int32 CenterY = 4 + Random2;
    FVector ZombieSpawnPos = GridManager->GetCellCenterWorldPos(CenterX, CenterY);
    ANPC* Zombie = GetWorld()->SpawnActor<ANPC>(NPCClass, ZombieSpawnPos, FRotator::ZeroRotator, SpawnParams);
    if (Zombie)
    {
        Zombie->SetState(EState::Zombie);
        Zombie->GridPoint = FIntPoint(CenterX, CenterY);
        Zombie->ZombieManager = this;

        AllNPCs.Add(Zombie);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Zombie"));
    }
}

void AZombieManager::CheckWinCondition()
{
    // Get all human positions
    TArray<FIntPoint> HumanPositions = GetCurrentHumanPositions();
    if (HumanPositions.Num() == 0) return; // No humans left

    TArray<ANPC*> AllZombies = GetAllPotentialZombies();

    // Check if any zombie can path to a human
    for (ANPC* Zombie : AllZombies)
    {
        if (!Zombie) continue;

        const FIntPoint ZPos = Zombie->GridPoint;

        for (const FIntPoint& HPos : HumanPositions)
        {
            TArray<FGridNode> Path;
            if (GridManager->FindPath(FGridNode(ZPos.X, ZPos.Y), FGridNode(HPos.X, HPos.Y), Path))
            {
                // At least one zombie can reach a human
                return;
            }
        }
    }

    // No zombies can reach any human
    bWinCon = true;
}

TArray<ANPC*> AZombieManager::GetAllPotentialZombies()
{
    TArray<ANPC*> AllZombies;
    for (ANPC* NPC : AllNPCs)
    {
        if (NPC->GetState() == EState::Zombie || NPC->GetState() == EState::Bitten)
        {
            AllZombies.Add(NPC);
        }
    }

    return AllZombies;
}

void AZombieManager::ExecuteTurn()
{
    UpdateBittenTimers();

    TArray<ANPC*> ActiveZombies = GetShuffledZombies();

    for (ANPC* Z : ActiveZombies)
    {
        if (AllowedBitesThisTurn <= 0) break;

        TryMoveAndBite(Z);

        AllowedBitesThisTurn--;
    }
    CheckWinCondition();
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

TArray<ANPC*> AZombieManager::GetShuffledZombies() const
{
    TArray<ANPC*> Zombies;
    for (ANPC* NPC : AllNPCs)
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
    for (ANPC* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human)
            Humans.Add(NPC->GridPoint);
    }
    return Humans;
}

ANPC* AZombieManager::GetHumanAtGridPos(const FIntPoint& Pos) const
{
    for (ANPC* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human && NPC->GridPoint == Pos)
            return NPC;
    }
    return nullptr;
}

bool AZombieManager::TryMoveAndBite(ANPC* Zombie)
{
    if (!GridManager || !Zombie) return false;

    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return false;

    FIntPoint ZPos = Zombie->GridPoint;

    TArray<FIntPoint> ClosestHumans;
    int32 BestDist = MAX_int32;

    for (FIntPoint HPos : Humans)
    {
        TArray<FGridNode> Path;
        FGridNode Start(HPos.X, HPos.Y);
        FGridNode End(HPos.X, HPos.Y);

        // Path from Zombie to Human
        if (!GridManager->FindPath(FGridNode(ZPos.X, ZPos.Y), FGridNode(HPos.X, HPos.Y), Path)) 
        {
            continue;
        }

        int32 Dist = Path.Num();
        if (Dist < BestDist)
        {
            BestDist = Dist;
            ClosestHumans.Empty();
            ClosestHumans.Add(HPos);
        }
        else if (Dist == BestDist)
        {
            ClosestHumans.Add(HPos);
        }
    }

    if (ClosestHumans.Num() == 0) return false;

    int32 RandIndex = FMath::RandRange(0, ClosestHumans.Num() - 1);
    FIntPoint Best = ClosestHumans[RandIndex];

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

    // set the new postion
    FGridNode LastNode = Path[Path.Num() - 1];
    Zombie->GridPoint = FIntPoint(LastNode.X, LastNode.Y);

    // give the zombie a movement path
    Zombie->MoveAlongWorldPath(VPath);

    ANPC* Human = GetHumanAtGridPos(Best);
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

void AZombieManager::NotifyZombieStartedMoving()
{
    ActiveMovingZombies++;
    bZombiesAreMoving = true;
}

void AZombieManager::NotifyZombieFinishedMoving()
{
    ActiveMovingZombies = FMath::Max(0, ActiveMovingZombies - 1);

    if (ActiveMovingZombies == 0)
    {
        bZombiesAreMoving = false;
    }
}


int32 AZombieManager::GetSusceptibleCount() const
{
    int32 Count = 0;
    for (ANPC* NPC : AllNPCs)
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
    for (ANPC* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Zombie)
            Count++;
    }
    return Count;
}
