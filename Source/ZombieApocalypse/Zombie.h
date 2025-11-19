// Copyright University of Inland Norway. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

UCLASS()
class ZOMBIEAPOCALYPSE_API AZombie : public ACharacter
{
    GENERATED_BODY()

public:
    AZombie();

    UFUNCTION(BlueprintImplementableEvent, Category = "Zombie")
    void OnMoveAlongPath(const TArray<FIntPoint>& GridPath);
};