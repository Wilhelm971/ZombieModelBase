#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "EnemyManager.generated.h"

// Forward Decls
class UGridManager;
class AZombie;
class AHuman;

// Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameWin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameOver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZombieTurned, FIntPoint, TurnedPos);

USTRUCT(BlueprintType)
struct FBittenHumanData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FIntPoint Position;

    UPROPERTY(BlueprintReadOnly)
    AHuman* HumanActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    int32 TurnsUntilTurn = 15;  // Configurable

    UPROPERTY(BlueprintReadOnly)
    bool bIsTurning = false;
};

UCLASS(Blueprintable, BlueprintType)
class YOURGAME_API AEnemyManager : public AActor
{
    GENERATED_BODY()

public:
    AEnemyManager();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void InitializeManager(UGridManager* InGridManager, TArray<AZombie*> InZombies, int32 InZombiesPerTurn);

    UFUNCTION(BlueprintCallable, Category = "Enemy Phase")
    void ExecuteZombiePhase();

    UFUNCTION(BlueprintCallable, Category = "Enemy Phase")
    bool CheckEndGameConditions();

    // Getters
    UFUNCTION(BlueprintPure, Category = "EnemyManager")
    TArray<FIntPoint> GetAllZombiePositions() const;

    UFUNCTION(BlueprintPure, Category = "EnemyManager")
    TArray<FBittenHumanData> GetBittenHumans() const { return BittenHumans; }

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetSavedHumansScore() const { return SavedHumansCount * ScoreMultiplier; }

    // Config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    int32 ZombiesPerTurn = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    int32 BittenTurnDelay = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    int32 ScoreMultiplier = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    TSubclassOf<AZombie> ZombieClass;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGameWin OnGameWin;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGameOver OnGameOver;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnZombieTurned OnZombieTurned;

    // Movement (called by BP after path received)
    UFUNCTION(BlueprintImplementableEvent, Category = "Zombie Movement")
    void OnMoveZombieToPath(AZombie* Zombie, const TArray<FIntPoint>& Path);

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    UGridManager* GridManager;

    // All zombie actors & their grid positions
    UPROPERTY()
    TArray<AZombie*> AllZombies;

    UPROPERTY()
    TArray<FIntPoint> ZombiePositions;

    // Bitten humans only (no unbitten tracking - GridManager or GameMode handles humans)
    UPROPERTY()
    TArray<FBittenHumanData> BittenHumans;

    UPROPERTY()
    int32 SavedHumansCount = 0;  // Set in Init or GameMode

private:
    // Phase Logic
    TArray<AZombie*> GetShuffledZombiesSubset() const;
    bool TryBiteClosestHuman(AZombie* Zombie);
    void UpdateBittenHumanTimers();
    void TurnBittenHumanIntoZombie(const FBittenHumanData& BittenData);
    FIntPoint GetZombieGridPos(const AZombie* Zombie) const;

    // Helpers
    UPROPERTY()
    TArray<FIntPoint> CurrentUnbittenHumanPositions;  // Filled by GridManager query each phase
};