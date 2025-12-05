// Copyright University of Inland Norway. All Rights Reserved.
#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NonPlayerCharacters.h"
#include "ZombieManager.generated.h"


USTRUCT()
struct FBittenNPC
{
	GENERATED_BODY()


	FIntPoint GridPos;
	ANonPlayerCharacters* NPC;
	int32 TurnsLeft = 15;
};


UCLASS()
class ZOMBIEAPOCALYPSE_API AZombieManager : public AActor
{
	GENERATED_BODY()


public:
	AZombieManager();


	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;


private:
	// NPC tracking
	UPROPERTY()
	TArray<ANonPlayerCharacters*> AllNPCs;


	// Bitten list
	UPROPERTY()
	TArray<FBittenNPC> BittenNPCs;


	// Config
	UPROPERTY(EditAnywhere)
	int32 ZombiesPerTurn = 3;


	UPROPERTY(EditAnywhere)
	TSubclassOf<ANonPlayerCharacters> NPCClass;


	// Helpers
	FIntPoint WorldToGrid(const FVector& WorldPos) const;
	FVector GridToWorld(const FIntPoint& GridPos) const;


	bool CanZombieReachHuman(FIntPoint Start, FIntPoint End) const;
	TArray<FIntPoint> BuildPath(FIntPoint Start, FIntPoint Goal) const;


	TArray<FIntPoint> GetCurrentHumanPositions() const;
	ANonPlayerCharacters* GetHumanAtGridPos(FIntPoint Pos) const;


	TArray<ANonPlayerCharacters*> GetShuffledZombies() const;


	bool TryMoveAndBite(ANonPlayerCharacters* Zombie);


	void UpdateBittenTimers();
	void TurnHumanIntoZombie(const FBittenNPC& Data);
};