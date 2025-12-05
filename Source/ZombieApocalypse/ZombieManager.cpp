// ZombieManager.cpp

#include "ZombieManager.h"
#include "Kismet/GameplayStatics.h"

AZombieManager::AZombieManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AZombieManager::BeginPlay()
{
    Super::BeginPlay();

    // Find all NPCs in world
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANonPlayerCharacters::StaticClass(), (TArray<AActor*>&)AllNPCs);
}

void AZombieManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBittenTimers();

    // Choose zombies this turn
    TArray<ANonPlayerCharacters*> ActiveZombies = GetShuffledZombies();

    for (ANonPlayerCharacters* Z : ActiveZombies)
    {
        TryMoveAndBite(Z);
    }
}

FIntPoint AZombieManager::WorldToGrid(const FVector& WorldPos) const
{
    return FIntPoint(FMath::RoundToInt(WorldPos.X / 100.f), FMath::RoundToInt(WorldPos.Y / 100.f));
}

FVector AZombieManager::GridToWorld(const FIntPoint& GridPos) const
{
    return FVector(GridPos.X * 100.f, GridPos.Y * 100.f, 0.f);
}

TArray<FIntPoint> AZombieManager::GetCurrentHumanPositions() const
{
    TArray<FIntPoint> Results;

    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Human)
        {
            Results.Add(WorldToGrid(NPC->GetActorLocation()));
        }
    }

    return Results;
}

ANonPlayerCharacters* AZombieManager::GetHumanAtGridPos(FIntPoint Pos) const
{
    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (!NPC) continue;

        if (NPC->GetState() == EState::Human)
        {
            if (WorldToGrid(NPC->GetActorLocation()) == Pos)
                return NPC;
        }
    }
    return nullptr;
}

TArray<ANonPlayerCharacters*> AZombieManager::GetShuffledZombies() const
{
    TArray<ANonPlayerCharacters*> Zombies;

    for (ANonPlayerCharacters* NPC : AllNPCs)
    {
        if (NPC && NPC->GetState() == EState::Zombie)
            Zombies.Add(NPC);
    }

    // Shuffle
    for (int32 i = Zombies.Num() - 1; i > 0; --i)
    {
        int32 j = FMath::RandRange(0, i);
        Zombies.Swap(i, j);
    }

    Zombies.SetNum(FMath::Min(ZombiesPerTurn, Zombies.Num()));

    return Zombies;
}

bool AZombieManager::CanZombieReachHuman(FIntPoint Start, FIntPoint End) const
{
    return true; // Placeholder for BFS reachability check
}

TArray<FIntPoint> AZombieManager::BuildPath(FIntPoint Start, FIntPoint Goal) const
{
    TArray<FIntPoint> Path;
    Path.Add(Start);
    Path.Add(Goal);
    return Path;
}

bool AZombieManager::TryMoveAndBite(ANonPlayerCharacters* Zombie)
{
    TArray<FIntPoint> Humans = GetCurrentHumanPositions();
    if (Humans.Num() == 0) return false;

    FIntPoint ZPos = WorldToGrid(Zombie->GetActorLocation());

    FIntPoint Best(-1, -1);
    int32 BestDist = MAX_int32;

    for (FIntPoint HPos : Humans)
    {
        if (!CanZombieReachHuman(ZPos, HPos)) continue;

        int32 Dist = FMath::Abs(HPos.X - ZPos.X) + FMath::Abs(HPos.Y - ZPos.Y);

        if (Dist < BestDist)
        {
            BestDist = Dist;
            Best = HPos;
        }
    }

    if (Best == FIntPoint(-1, -1)) return false;

    // Path
    TArray<FIntPoint> Path = BuildPath(ZPos, Best);
    if (Path.Num() < 2) return false;

    // MOVE HERE MOVE HERE MOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HEREMOVE HERE MOVE HERE
    //Zombie->OnMoveAlongPath(Path);

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

void AZombieManager::UpdateBittenTimers()
{
    for (int32 i = BittenNPCs.Num() - 1; i >= 0; --i)
    {
        FBittenNPC& B = BittenNPCs[i];

        if (--B.TurnsLeft <= 0)
        {
            TurnHumanIntoZombie(B);
            BittenNPCs.RemoveAt(i);
        }
    }
}

void AZombieManager::TurnHumanIntoZombie(const FBittenNPC& Data)
{
    if (!Data.NPC) return;

    Data.NPC->SetState(EState::Zombie);
}
