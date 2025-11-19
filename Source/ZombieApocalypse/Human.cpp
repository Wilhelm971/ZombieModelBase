#include "AHuman.h"

AHuman::AHuman()
{
    PrimaryActorTick.bCanEverTick = false;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HumanMesh"));
    RootComponent = Mesh;
    // Assign any cube/sphere/cylinder in BP later
}