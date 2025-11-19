#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AHuman.generated.h"

UCLASS()
class YOURGAME_API AHuman : public AActor
{
    GENERATED_BODY()
public:
    AHuman();

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;
};