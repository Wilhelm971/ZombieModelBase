// Copyright University of Inland Norway. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieManager.generated.h"

class AGridManager;
class AZombie;
class AHuman;

USTRUCT(BlueprintType)
struct FBittenHuman
{
    GENERATED_BODY()

    UPROPERTY() FIntPoint GridPos;
    UPROPERTY() AHuman* HumanActor = nullptr;
    UPROPERTY() int32 TurnsLeft = 15;
};

UCLASS()
class ZOMBIEAPOCALYPSE_API AZombieManager : public AActor
{
    GENERATED_BODY()

public:
    AZombieManager();

    UFUNCTION(BlueprintCallable, Category = "Zombie System")
    void Initialize(AGridManager* InGridManager, const TArray<AZombie*>& StartingZombies);

    UFUNCTION(BlueprintCallable, Category = "Zombie System")
    void ExecuteZombiePhase();

    // Returns true = WIN (all humans safe + no bitten)
    UFUNCTION(BlueprintCallable, Category = "Game State")
    bool IsWinConditionMet() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TSubclassOf<AZombie> ZombieClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 ZombiesPerTurn = 3;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY() TObjectPtr<AGridManager> GridManager;

    UPROPERTY() TArray<AZombie*> AllZombies;
    UPROPERTY() TArray<FBittenHuman> BittenHumans;

    // === Helper functions that work with your exact GridManager ===
    FIntPoint WorldToGrid(FVector WorldLocation) const;
    FVector GridToWorld(FIntPoint GridPos) const;
    bool CanZombieReachHuman(FIntPoint ZombieGrid, FIntPoint HumanGrid) const;
    TArray<FIntPoint> ReconstructPathFromBFS(const TMap<FIntPoint, FIntPoint>& CameFrom, FIntPoint End) const;

    TArray<AZombie*> GetShuffledZombies() const;
    bool TryMoveAndBite(AZombie* Zombie);
    void UpdateBittenTimers();
    void TurnHumanIntoZombie(const FBittenHuman& Data);

    // You must implement these two in your GameMode or GridManager
    TArray<FIntPoint> GetCurrentHumanPositions() const;      // ← You fill this
    AHuman* GetHumanAtGridPos(FIntPoint Pos) const;          // ← You fill this
};