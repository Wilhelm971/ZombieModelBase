// Copyright University of Inland Norway. All Rights Reserved.

#include "Human.h"

AHuman::AHuman()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HumanMesh"));
    RootComponent = MeshComponent;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Cylinder.Succeeded())
    {
        MeshComponent->SetStaticMesh(Cylinder.Object);
        MeshComponent->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.6f));
    }
}