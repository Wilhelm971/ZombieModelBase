#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPC.h"
#include "GridManager.h"
#include "ZombieManager.generated.h"

USTRUCT()
struct FBittenNPC
{
    GENERATED_BODY()

    UPROPERTY()
    ANPC* NPC;

    UPROPERTY()
    FIntPoint GridPos;

    UPROPERTY()
    int32 TurnsLeft;
};

UCLASS()
class ZOMBIEAPOCALYPSE_API AZombieManager : public AActor
{
    GENERATED_BODY()

public:
    AZombieManager();

    // Reference to NPC class for spawning all units
    UPROPERTY(EditAnywhere, Category = "Zombies")
    TSubclassOf<ANPC> NPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AGridManager* GridManager;

    //Set by the Simulation
    UPROPERTY()
    int32 AllowedBitesThisTurn = 0;

    UFUNCTION()
    void SpawnInitialNPCs();

    // Called by TurnManager each turn
    UFUNCTION()
    void ExecuteTurn();

    //Getters for the Simulation
    UFUNCTION(BlueprintCallable)
    int32 GetSusceptibleCount() const;

    UFUNCTION(BlueprintCallable)
    int32 GetBittenCount() const;

    UFUNCTION(BlueprintCallable)
    int32 GetZombieCount() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<ANPC*> AllNPCs;

    UPROPERTY()
    TArray<FBittenNPC> BittenNPCs;

    // Turn-based helpers
    void UpdateBittenTimers();
    bool TryMoveAndBite(ANPC* Zombie);
    TArray<ANPC*> GetShuffledZombies() const;
    TArray<FIntPoint> GetCurrentHumanPositions() const;
    ANPC* GetHumanAtGridPos(const FIntPoint& Pos) const;
};
