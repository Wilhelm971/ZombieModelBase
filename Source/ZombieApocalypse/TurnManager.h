#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurnManager.generated.h"


UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
	PlayerTurn,
	ZombieTurn
};

UCLASS()
class ZOMBIEAPOCALYPSE_API ATurnManager : public AActor
{
	GENERATED_BODY()

public:
	ATurnManager();


	UPROPERTY(BlueprintReadOnly)
	ETurnPhase CurrentPhase = ETurnPhase::PlayerTurn;

	UFUNCTION(BlueprintCallable)
	void EndPlayerTurn();

private:
	void ProcessZombieTurn();
	void MoveAllZombies();
	void ZombieAttacks();
};