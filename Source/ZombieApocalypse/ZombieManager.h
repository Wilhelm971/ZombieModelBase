#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NonPlayerCharacters.h"
#include "GridManager.h"
#include "ZombieManager.generated.h"

USTRUCT()
struct FBittenNPC
{
    GENERATED_BODY()

    UPROPERTY()
    ANonPlayerCharacters* NPC;

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
    TSubclassOf<ANonPlayerCharacters> NPCClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AGridManager* GridManager;

    UFUNCTION()
    void SpawnInitialNPCs();

    // Called by TurnManager each turn
    UFUNCTION()
    void ExecuteTurn();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<ANonPlayerCharacters*> AllNPCs;

    UPROPERTY()
    TArray<FBittenNPC> BittenNPCs;

    // Turn-based helpers
    void UpdateBittenTimers();
    bool TryMoveAndBite(ANonPlayerCharacters* Zombie);
    TArray<ANonPlayerCharacters*> GetShuffledZombies() const;
    TArray<FIntPoint> GetCurrentHumanPositions() const;
    ANonPlayerCharacters* GetHumanAtGridPos(const FIntPoint& Pos) const;
};
