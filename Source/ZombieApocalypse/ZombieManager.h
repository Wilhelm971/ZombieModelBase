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

    UPROPERTY(EditAnywhere, Category = "Zombies")
    TSubclassOf<ANonPlayerCharacters> HumanClass;

    UPROPERTY(EditAnywhere, Category = "Zombies")
    TSubclassOf<ANonPlayerCharacters> ZombieClass;

    UPROPERTY(EditAnywhere, Category = "Zombies")
    AGridManager* GridManager;

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

    // Helpers
    TArray<FIntPoint> GetCurrentHumanPositions() const;
    ANonPlayerCharacters* GetHumanAtGridPos(const FIntPoint& Pos) const;
    bool TryMoveAndBite(ANonPlayerCharacters* Zombie);
    void UpdateBittenTimers();
};
