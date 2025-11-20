#include "TurnManager.h"
#include "Kismet/GameplayStatics.h"
#include "GridManager.h"

ATurnManager::ATurnManager()
{
    PrimaryActorTick.bCanEverTick = false; // turn-based: no ticking
}

void ATurnManager::EndPlayerTurn()
{
    CurrentPhase = ETurnPhase::ZombieTurn;
    ProcessZombieTurn();
    CurrentPhase = ETurnPhase::PlayerTurn;
}

void ATurnManager::ProcessZombieTurn()
{
    MoveAllZombies();
    ZombieAttacks();
}

void ATurnManager::MoveAllZombies()
{
    AGridManager* Grid = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!Grid) 
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveAllZombies: GridManager not found"));
        return;
    }

    for (int y = 0; y < AGridManager::GridSize; ++y)
    {
        for (int x = 0; x < AGridManager::GridSize; ++x)
        {
            if (Grid->Grid[Grid->GetGridIndex(x, y)].State == ECellState::Zombie)
            {
                FGridNode Start(x, y);

                // Find the nearest human (brute-force search)
                bool Found = false;
                FGridNode NearestHuman(0,0);
                TArray<FGridNode> BestPath;

                for (int yy = 0; yy < AGridManager::GridSize; ++yy)
                {
                    for (int xx = 0; xx < AGridManager::GridSize; ++xx)
                    {
                        if (Grid->Grid[Grid->GetGridIndex(xx, yy)].State == ECellState::Human)
                        {
                            TArray<FGridNode> Path;
                            if (Grid->FindPath(Start, FGridNode(xx, yy), Path))
                            {
                                if (!Found || Path.Num() < BestPath.Num())
                                {
                                    Found = true;
                                    BestPath = Path;
                                    NearestHuman = FGridNode(xx, yy);
                                }
                            }
                        }
                    }
                }

                if (Found && BestPath.Num() > 1)
                {
                    FGridNode NextStep = BestPath[1];

                    // Move zombie (note: this simple approach may cause multiple zombies to overwrite each other;
                    // see suggestions below to make moves atomic)
                    Grid->Grid[Grid->GetGridIndex(x,y)].State = ECellState::Empty;
                    Grid->Grid[Grid->GetGridIndex(NextStep.X,NextStep.Y)].State = ECellState::Zombie;
                }
            }
        }
    }
}

void ATurnManager::ZombieAttacks()
{
    AGridManager* Grid = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    if (!Grid) return;

    for (int y = 0; y < Grid->GridSize; ++y)
    {
        for (int x = 0; x < Grid->GridSize; ++x)
        {
            if (Grid->Grid[Grid->GetGridIndex(x,y)].State == ECellState::Zombie)
            {
                static const int32 Dx[4] = { -1, 1, 0, 0 };
                static const int32 Dy[4] = { 0, 0, -1, 1 };

                for (int i = 0; i < 4; ++i)
                {
                    int nx = x + Dx[i];
                    int ny = y + Dy[i];

                    if (!Grid->IsValidCell(nx, ny)) continue;

                    if (Grid->Grid[Grid->GetGridIndex(nx,ny)].State == ECellState::Human)
                    {
                        Grid->Grid[Grid->GetGridIndex(nx,ny)].State = ECellState::Zombie;
                    }
                }
            }
        }
    }
}