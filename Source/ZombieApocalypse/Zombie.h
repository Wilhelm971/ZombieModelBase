#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AZombie.generated.h"

UCLASS()
class YOURGAME_API AZombie : public ACharacter
{
    GENERATED_BODY()
public:
    AZombie();

    // Called from EnemyManager → Blueprint does the actual movement
    UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
    void OnMoveZombieToPath(const TArray<FIntPoint>& Path);
};