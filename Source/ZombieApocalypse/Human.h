// Copyright University of Inland Norway. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Human.generated.h"

UCLASS()
class ZOMBIEAPOCALYPSE_API AHuman : public AActor
{
    GENERATED_BODY()

public:
    AHuman();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;
};