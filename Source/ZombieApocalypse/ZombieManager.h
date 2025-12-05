// Copyright University of Inland Norway. All Rights Reserved.
/*
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NonPlayerCharacters.h"
#include "GridManager.h"
#include "ZombieManager.generated.h"

USTRUCT()
struct FBittenEntry
{
    GENERATED_BODY()

    FIntPoint GridPos;
    ABeing* HumanActor = nullptr;
    int32 TurnsLeft = 15;
};

UCLASS()
class YOURGAME_API AZombieManager : public AActor
{
    GENERATED_BODY()

public:
    AZombieManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ABeing> BeingClass;

    UPROPERTY(BlueprintReadWrite)
    AGridManager* GridManager;

    UPROPERTY(BlueprintReadWrite)
    TArray<ABeing*> AllBeings;    // All humans AND zombies

    UPROPERTY(EditAnywhere)
    int32 ZombiesPerTurn = 5;

    UPROPERTY()
    TArray<FBittenEntry> BittenList;

    virtual void BeginPlay() override;

    void Initialize(AGridManager* InGrid);

    void SpawnInitialPopulation();

    void ExecuteZombieTurn();

    bool IsWinConditionMet() const;

private:

    // ===== Core Helpers =====
    FVector GridToWorld(FIntPoint Grid) const;
    FIntPoint WorldToGrid(FVector World) const;

    //TArray<ABeing*> GetZombies() const;
    TArray<FIntPoint> GetHumanPositions() const;

    ABeing* GetHumanAt(FIntPoint GridPos) const;

    bool TryMoveAndBite(ABeing* Zombie);

    bool CanReachHuman(FIntPoint Start, FIntPoint Goal) const;

    bool BuildBFSPath(FIntPoint Start, FIntPoint Goal, TArray<FIntPoint>& OutPath) const;

    void UpdateBitten();
    void TurnHumanIntoZombie(const FBittenEntry& Data);
};
*/